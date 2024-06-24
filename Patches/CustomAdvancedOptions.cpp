/**
* Copyright (C) 2024 Murugo
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>
#include <vector>
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"
#include "Patches\Patches.h"
#include "Resolution.h"

extern char* getRenderResolutionStr();
extern char* getRenderResolutionDescriptionStr();

namespace
{
    constexpr BYTE kAdvancedOptionsPage = 0x07;

    bool DisplayChangeRequired = false;
    bool SaveConfigRequired = false;

    DWORD* MsgFileAddr = nullptr;
    BYTE* OptionsPage = nullptr;
    BYTE* OptionsState = nullptr;
    short* OptionsSelectIndex = nullptr;
    short* OptionsSelectDeltaIndex = nullptr;
    BYTE* OptionsSaveChangesActive = nullptr;
    BYTE* OptionsSaveChangesIndex = nullptr;
    BYTE* OptionCountAddr = nullptr;
    void* ScreenBrightnessHandlerAddr = nullptr;
    void* jmpHandleInputAddr = nullptr;
    void* jmpSkipDisabledOptionsReturnAddr1 = nullptr;
    void* jmpSkipDisabledOptionsReturnAddr2 = nullptr;
    void* jmpCheckChangedOptionsReturnAddr = nullptr;
    void* jmpSaveOrDiscardChangesReturnAddr1 = nullptr;
    void* jmpSaveOrDiscardChangesReturnAddr2 = nullptr;

    BYTE* NoiseEffectOptionValue = nullptr;
    BYTE* NoiseEffectOptionCommitValue = nullptr;
    BYTE* AdvancedFiltersAvailable = nullptr;
    BYTE* LensFlareAvailable = nullptr;
    BYTE* DxConfigShadowsEnabled = nullptr;
    BYTE* DxConfigFogEnabled = nullptr;
    BYTE* DxConfigMotionBlurEnabled = nullptr;
    BYTE* DxConfigDepthOfFieldEnabled = nullptr;
    BYTE* DxConfigLensFlareEnabled = nullptr;
    BYTE* DxConfigResolution = nullptr;
    void(*HandleAdvancedFiltersChange)();

    char*(*prepText)(const char *str);
    void(*printText)(const char *str, int x, int y);
    void(*printTextMsg)(const short* msg_file, short index, int x, int y);
    void(*textSetColor)(BYTE r, BYTE g, BYTE b, BYTE a);
    void(*textSetCenterAlign)();
    void(*textUnsetCenterAlign)();
    void(*textSetRightAlign)();
    void(*textUnsetRightAlign)();
    void(*drawLeftSelectArrow)(int x, int y);
    void(*drawRightSelectArrow)(int x, int y);
    int(*getTextWidth)(int* len, USHORT* str);
    bool(*joyDown)(DWORD port, DWORD key1, DWORD key2);
    bool(*keyDown)(DWORD key);
    void(*playSound)(int index, float volume, int pan);
    bool(*mouseDown)(DWORD key);
    int(*mouseX)();
    int(*mouseY)();
    void(*updateResolution)(BYTE index);
    void(*updateRenderEffects)();

    void SetTextColorUnselected() { textSetColor(0x3F, 0x3F, 0x3F, 0x60); }
    void SetTextColorUnselectedChanged() { textSetColor(0x3F, 0x3F, 0x1F, 0x60); }
    void SetTextColorSelected() { textSetColor(0x3F, 0x3F, 0x3F, 0xFF); }
    void SetTextColorSelectedChanged() { textSetColor(0x3F, 0x3F, 0x1F, 0xFF); }
    void SetTextColorDisabled() { textSetColor(0x33, 0x33, 0x33, 0x60); }

    class OptionText
    {
    public:
        virtual void Draw(int x, int y) const = 0;
        virtual int GetTextWidth() const = 0;
    };

    class OptionRawText : public OptionText
    {
    public:
        OptionRawText(const char* text) : text_(text) {}

        void Draw(int x, int y) const override
        {
            if (text_ == nullptr) return;
            printText(prepText(text_), x, y);
        }

        int GetTextWidth() const override
        {
            int len = 0;
            getTextWidth(&len, (USHORT*)(prepText(text_)));
            return len;
        }

    private:
        const char* text_;
    };

    class OptionMsgText : public OptionText
    {
    public:
        OptionMsgText(short msg_index) : msg_index_(msg_index) {}

        void Draw(int x, int y) const override
        {
            if (msg_index_ <= 0) return;
            printTextMsg((const short*)*MsgFileAddr, msg_index_, x, y);
        }

        int GetTextWidth() const override
        {
            if (msg_index_ <= 0) return 0;
            int len = 0;
            USHORT* msg_ptr = (USHORT*)(*MsgFileAddr);
            USHORT* text_ptr = msg_ptr + msg_ptr[msg_index_ + 1];
            getTextWidth(&len, text_ptr);
            return len;
        }

    private:
        const short msg_index_;
    };

    class Option
    {
    public:
        Option(int index, std::unique_ptr<OptionText> name, std::unique_ptr<OptionText> description) :
            index_(index),
            name_(std::move(name)),
            description_(std::move(description)),
            enabled_(true)
        {
            option_y_ = index * 0x1B + 0x64;
        }

        void AddValue(std::unique_ptr<OptionText> value)
        {
            values_.push_back(std::move(value));
        }

        void SetDisabledValue(std::unique_ptr<OptionText> value)
        {
            value_disabled_ = std::move(value);
        }

        const std::vector<std::unique_ptr<OptionText>>& Values() const
        {
            return values_;
        }

        int ValueIndex() const
        {
            return value_index_;
        }

        virtual void Init() {}

        virtual void Draw() const
        {
            if (*OptionsSaveChangesActive > 0 && index_ > 2 && index_ < 7)
            {
                if (*OptionsSelectIndex == index_ && enabled_)
                    DrawDescription();

                return;
            }

            DrawBase();
            if (*OptionsSelectIndex == index_ && enabled_)
            {
                DrawHighlight();
                DrawDescription();
                DrawSelectArrows();
            }
        }

        virtual void Run() {}

        virtual bool Apply()
        {
            if (value_index_ != committed_value_index_)
            {
                committed_value_index_ = value_index_;
                return true;
            }
            return false;
        }

        virtual void Reset()
        {
            value_index_ = committed_value_index_;
        }

        virtual bool IsChanged()
        {
            return !values_.empty() && value_index_ != committed_value_index_;
        }

        virtual void NextValue()
        {
            if (!enabled_ || values_.empty()) return;
            if (++value_index_ >= (int)values_.size())
                value_index_ = 0;
            playSound(10000, 1.0, 0);
        }

        virtual void PreviousValue()
        {
            if (!enabled_ || values_.empty()) return;
            if (--value_index_ < 0)
                value_index_ = values_.size() - 1;
            playSound(10000, 1.0, 0);
        }

        virtual void HandleMouseSelection()
        {
            if (!enabled_) return;

            const int x = mouseX();
            const int y = mouseY();

            if (y < index_ * 0x1B + 0x53 || y > index_ * 0x1B + 0x6E)
                return;

            if (values_.empty())
            {
                if (name_ == nullptr) return;
                const int text_half_width = name_->GetTextWidth() / 2;
                if (x > 0xFC - text_half_width && x < 0xFC + text_half_width)
                    Run();
            }
            else
            {
                if (x >= 0xF1 && x <= 0x10A)
                {
                    PreviousValue();
                    return;
                }
                if (value_index_ < 0 || (size_t)value_index_ >= values_.size()) return;
                const int text_width = values_[value_index_]->GetTextWidth();
                if (x > text_width + 0x104 && x < text_width + 0x11F)
                    NextValue();
            }
        }

        virtual bool Enabled() const { return enabled_; }

        Option(const Option&) = delete;
        Option& operator=(const Option&) = delete;

    protected:
        virtual void DrawBase() const
        {
            if (!enabled_)
                SetTextColorDisabled();
            else
                SetTextColorUnselected();
                
            if (!values_.empty())
            {
                textUnsetCenterAlign();
                textSetRightAlign();
                name_->Draw(0xF3, option_y_);
                textUnsetRightAlign();
                if (committed_value_index_ != value_index_)
                    SetTextColorUnselectedChanged();
                if (enabled_)
                    values_[value_index_]->Draw(0x10E, option_y_);
                else if (value_disabled_ != nullptr)
                    value_disabled_->Draw(0x10E, option_y_);
            }
            else
            {
                textSetCenterAlign();
                textUnsetRightAlign();
                name_->Draw(0x100, option_y_);
            }
        }

        virtual void DrawHighlight() const
        {
            if (!enabled_) return;
            SetTextColorSelected();
            if (!values_.empty())
            {
                textUnsetCenterAlign();
                textSetRightAlign();
                name_->Draw(0xF1, option_y_ - 0x02);
                textUnsetRightAlign();
                if (committed_value_index_ != value_index_)
                    SetTextColorSelectedChanged();
                values_[value_index_]->Draw(0x10C, option_y_ - 0x02);
            }
            else
            {
                textSetCenterAlign();
                textUnsetRightAlign();
                name_->Draw(0xFE, option_y_ - 0x02);
            }
        }

        virtual void DrawSelectArrows() const
        {
            if (*OptionsSaveChangesActive > 0) return;
            if (values_.empty() || value_index_ < 0 || (size_t)value_index_ >= values_.size()) return;
            const int y = index_ * 0x1B - 0x8C;
            drawLeftSelectArrow(-0x05, y);
            const OptionText* value_text = values_[value_index_].get();
            if (value_text == nullptr) return;
            drawRightSelectArrow(value_text->GetTextWidth() + 0x1E, y);
        }

        virtual void DrawDescription() const
        {
            SetTextColorSelected();
            textUnsetCenterAlign();
            textUnsetRightAlign();
            description_->Draw(0x46, 0x18B);
        }

    protected:
        int index_;
        std::unique_ptr<OptionText> name_;
        std::unique_ptr<OptionText> description_;
        std::vector<std::unique_ptr<OptionText>> values_;
        std::unique_ptr<OptionText> value_disabled_;
        int value_index_ = 0;
        int committed_value_index_ = 0;
        bool enabled_;
        int option_y_;
    };

    class ScreenBrightnessOption : public Option
    {
    public:
        ScreenBrightnessOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionMsgText>((short)0x02),
            /*description=*/std::make_unique<OptionMsgText>((short)0x03)
        ) {}

        void Run() override
        {
            __asm { call ScreenBrightnessHandlerAddr }
        }
    };

    class DisplayModeOption : public Option
    {
    public:
        DisplayModeOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionMsgText>((short)0xFE),
            /*description=*/std::make_unique<OptionMsgText>((short)0xFF)
        )
        {
            AddValue(std::make_unique<OptionMsgText>((short)0x100));  // "Windowe"
            AddValue(std::make_unique<OptionMsgText>((short)0x101));  // "FS Windowed"
            AddValue(std::make_unique<OptionMsgText>((short)0x102));  // "Full Screen"
        }

        void Init() override
        {
            value_index_ = committed_value_index_ = ConfigData.DisplayModeOption - 1;
        }

        bool Apply() override
        {
            if (!Option::Apply()) return false;
            ConfigData.DisplayModeOption = value_index_ + 1;
            ScreenMode = value_index_ + 1;
            DisplayChangeRequired = true;
            SaveConfigRequired = true;
            return true;
        }
    };

    class DisplayResolutionOption : public Option
    {
    public:
        DisplayResolutionOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionMsgText>((short)0xA8),
            /*description=*/std::make_unique<OptionMsgText>((short)0xCA)
        ) {}

        void Init() override
        {
            values_.clear();
            for (const auto& text : GetResolutionText())
            {
                AddValue(std::make_unique<OptionRawText>(text.resStrBuf));
            }
            value_index_ = committed_value_index_ = *DxConfigResolution;
        }

        bool Apply() override
        {
            if (!Option::Apply()) return false;
            DisplayChangeRequired = true;
            WSFDynamicChangeWithResIndex((BYTE)value_index_);
            return true;
        }
    };

    Option* DisplayResolutionOptionPtr = nullptr;

    static const std::pair<int, const char*> kRenderResolutionScaleArray[]{
        {7, "\\hx0.25"},
        {6, "\\hx0.5"},
        {5, "\\hx0.75"},
        {0, "\\hx1.0"},
        {4, "\\hx1.25"},
        {3, "\\hx1.5"},
        {2, "\\hx1.75"},
        {1, "\\hx2.0"},
    };

    class RenderResolutionOption : public Option
    {
    public:
        RenderResolutionOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionRawText>(getRenderResolutionStr()),
            /*description=*/std::make_unique<OptionRawText>(getRenderResolutionDescriptionStr())
        )
        {
            for (const auto& [ind, text] : kRenderResolutionScaleArray)
            {
                AddValue(std::make_unique<OptionRawText>(text));
            }
        }

        void Init() override
        {
            for (int i = 0; i < sizeof(kRenderResolutionScaleArray); ++i)
            {
                if (kRenderResolutionScaleArray[i].first == (int)ConfigData.ScaleWindowedResolutionOption)
                {
                    value_index_ = committed_value_index_ = i;
                    return;
                }
            }
        }

        bool Apply() override
        {
            if (!Option::Apply()) return false;
            if (DisplayResolutionOptionPtr == nullptr) return false;
            ConfigData.ScaleWindowedResolutionOption = kRenderResolutionScaleArray[value_index_].first;
            SaveConfigRequired = true;

            // TODO: Update scale resolution here.
            // DisplayChangeRequired = true;
            // WSFDynamicChangeWithResIndex((BYTE)DisplayResolutionOptionPtr->ValueIndex());
            return true;
        }
    };

    class NoiseEffectOption : public Option
    {
    public:
        NoiseEffectOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionMsgText>((short)0x61),
            /*description=*/std::make_unique<OptionMsgText>((short)0x62)
        )
        {
            AddValue(std::make_unique<OptionMsgText>((short)0x45));  // "Off"
            AddValue(std::make_unique<OptionMsgText>((short)0x44));  // "On"
        }

        void Init() override
        {
            value_index_ = committed_value_index_ = *NoiseEffectOptionValue;
        }

        bool Apply() override
        {
            if (!Option::Apply()) return false;
            *NoiseEffectOptionValue = *NoiseEffectOptionCommitValue = (BYTE)value_index_;
            return true;
        }
    };

    class ShadowsOption : public Option
    {
    public:
        ShadowsOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionMsgText>((short)0xAF),
            /*description=*/std::make_unique<OptionMsgText>((short)0xCB)
        )
        {
            AddValue(std::make_unique<OptionMsgText>((short)0xB0));  // "Off"
            AddValue(std::make_unique<OptionMsgText>((short)0xB1));  // "On"
        }

        void Init() override
        {
            value_index_ = committed_value_index_ = *DxConfigShadowsEnabled;
        }

        bool Apply() override
        {
            if (!Option::Apply()) return false;
            *DxConfigShadowsEnabled = (BYTE)value_index_;
            return true;
        }
    };

    class FogOption : public Option
    {
    public:
        FogOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionMsgText>((short)0xB2),
            /*description=*/std::make_unique<OptionMsgText>((short)0xCC)
        )
        {
            AddValue(std::make_unique<OptionMsgText>((short)0xB3));  // "Simple"
            AddValue(std::make_unique<OptionMsgText>((short)0xB4));  // "Complex"
        }

        void Init() override
        {
            value_index_ = committed_value_index_ = *DxConfigFogEnabled;
        }

        bool Apply() override
        {
            if (!Option::Apply()) return false;
            *DxConfigFogEnabled = (BYTE)value_index_;
            return true;
        }
    };

    class AdvancedFiltersOption : public Option
    {
    public:
        AdvancedFiltersOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionMsgText>((short)0xB5),
            /*description=*/std::make_unique<OptionMsgText>((short)0xCD)
        )
        {
            AddValue(std::make_unique<OptionMsgText>((short)0xB0));  // "Off"
            AddValue(std::make_unique<OptionMsgText>((short)0xB1));  // "On"
            SetDisabledValue(std::make_unique<OptionMsgText>((short)0xBC));  // "Not Available"
        }

        void Init() override
        {
            if (*AdvancedFiltersAvailable)
            {
                enabled_ = true;
                value_index_ = committed_value_index_ = (*DxConfigMotionBlurEnabled || *DxConfigDepthOfFieldEnabled) ? 1 : 0;
            }
            else
            {
                enabled_ = false;
                value_index_ = committed_value_index_ = 0;
            }
        }

        bool Apply() override
        {
            if (!Option::Apply()) return false;
            *DxConfigMotionBlurEnabled = *DxConfigDepthOfFieldEnabled = (BYTE)value_index_;
            HandleAdvancedFiltersChange();
            return true;
        }
    };

    class LensFlareOption : public Option
    {
    public:
        LensFlareOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionMsgText>((short)0xC0),
            /*description=*/std::make_unique<OptionMsgText>((short)0xD0)
        )
        {
            AddValue(std::make_unique<OptionMsgText>((short)0xB0));  // "Off"
            AddValue(std::make_unique<OptionMsgText>((short)0xB1));  // "On"
            SetDisabledValue(std::make_unique<OptionMsgText>((short)0xBC));  // "Not Available"
        }

        void Init() override
        {
            if (*LensFlareAvailable)
            {
                enabled_ = true;
                value_index_ = committed_value_index_ = *DxConfigLensFlareEnabled;
            }
            else
            {
                enabled_ = false;
                value_index_ = committed_value_index_ = 0;
            }
        }

        bool Apply() override
        {
            if (!Option::Apply()) return false;
            *DxConfigLensFlareEnabled = (BYTE)value_index_;
            return true;
        }
    };

    std::vector<std::unique_ptr<Option>> options;

    void CreateOptions()
    {
        int index = 0;
        options.push_back(std::make_unique<ScreenBrightnessOption>(index++));
        options.push_back(std::make_unique<DisplayModeOption>(index++));
        options.push_back(std::make_unique<DisplayResolutionOption>(index++));
        DisplayResolutionOptionPtr = options.back().get();
        options.push_back(std::make_unique< RenderResolutionOption>(index++));
        options.push_back(std::make_unique<NoiseEffectOption>(index++));
        options.push_back(std::make_unique<ShadowsOption>(index++));
        options.push_back(std::make_unique<FogOption>(index++));
        options.push_back(std::make_unique<AdvancedFiltersOption>(index++));
        options.push_back(std::make_unique<LensFlareOption>(index++));

        UpdateMemoryAddress((void*)(OptionCountAddr), &index, 1);
    }
    
    void HandleAdvancedOptionsInput()
    {
        if (*OptionsState != 0x00)
            return;

        const short select_index = *OptionsSelectIndex;
        if (select_index < 0 && select_index >= (int)options.size())
            return;

        Option* selected_option = options[select_index].get();
        if (!selected_option->Enabled())
            return;

        // Mouse selection
        if (mouseDown(0x01))
        {
            selected_option->HandleMouseSelection();
            return;
        }

        // Select submenu with keyboard/controller
        if (joyDown(0, 0x10, 0) || joyDown(0, 0x200, 0) || keyDown(0x10))
            selected_option->Run();

        // Increment/decrement options with keyboard/controller
        if (joyDown(0, 0x2000004, 0) || keyDown(0x04))
            selected_option->PreviousValue();
        else if (joyDown(0, 0x1000008, 0) || keyDown(0x08))
            selected_option->NextValue();
    }

    void DrawAdvancedOptions()
    {
        for (const auto& option : options)
        {
            option->Draw();
        }
        if (!*OptionsSaveChangesActive)
        {
            HandleAdvancedOptionsInput();
        }
    }

    __declspec(naked) void __stdcall DrawAdvancedOptionsASM()
    {
        __asm
        {
            call DrawAdvancedOptions
            pop edi
            pop ebp
            add esp, 0x30
            ret
        }
    }

    void SkipDisabledOptions()
    {
        short select_index = *OptionsSelectIndex;
        const short delta_index = *OptionsSelectDeltaIndex;
        const size_t option_count = options.size();

        if (select_index < 0)
            select_index = (short)option_count - 1;
        else if (select_index >= (int)option_count)
            select_index = 0;
        
        // Scan up/down to find the next enabled option
        for (size_t i = 0; i < option_count; ++i)
        {
            short index = (short)(((select_index + delta_index * i) % option_count) + option_count) % option_count;
            if (options[index]->Enabled())
            {
                *OptionsSelectIndex = index;
                return;
            }
        }
    }

    __declspec(naked) void __stdcall SkipDisabledOptionsASM()
    {
        __asm
        {
            push eax
            mov eax, dword ptr ds : [OptionsPage]
            mov al, byte ptr ds : [eax]
            cmp al, kAdvancedOptionsPage
            jnz ExitASM

            push ebx
            call SkipDisabledOptions
            pop ebx
            pop eax
            jmp jmpSkipDisabledOptionsReturnAddr1

        ExitASM:
            pop eax
            jmp jmpSkipDisabledOptionsReturnAddr2
        }
    }

    bool AnyOptionChanged()
    {
        for (const auto& option : options)
        {
            if (option->IsChanged())
                return true;
        }
        return false;
    }

    __declspec(naked) void __stdcall CheckChangedOptionsASM()
    {
        __asm
        {
            call AnyOptionChanged
            test eax,eax
            jmp jmpCheckChangedOptionsReturnAddr
        }
    }

    void SaveChanges()
    {
        for (auto& option : options)
        {
            option->Apply();
        }
        if (DisplayChangeRequired)
        {
            updateResolution((BYTE)DisplayResolutionOptionPtr->ValueIndex());
            updateRenderEffects();
            DisplayChangeRequired = false;
        }
        if (SaveConfigRequired)
        {
            SaveConfigData();
            SaveConfigRequired = false;
        }
    }

    void DiscardChanges()
    {
        for (auto& option : options)
        {
            option->Reset();
        }
    }

    __declspec(naked) void __stdcall SaveOrDiscardChangesASM()
    {
        __asm
        {
            mov eax, dword ptr ds : [OptionsSaveChangesIndex]
            mov al, byte ptr ds : [eax]
            test al, al
            jnz Discard
            
            call SaveChanges
            jmp jmpSaveOrDiscardChangesReturnAddr1

        Discard:
            call DiscardChanges
            jmp jmpSaveOrDiscardChangesReturnAddr2
        }
    }

    void InitOptions()
    {
        for (auto& option : options)
        {
            option->Init();
        }
    }

    _declspec(naked) void __stdcall InitOptionsASM()
    {
        __asm
        {
            push esi
            call InitOptions
            pop esi
            mov eax, dword ptr ds : [OptionsSelectIndex]
            mov word ptr ds : [eax], si
            ret
        }
    }
}

void PatchCustomAdvancedOptions()
{
    constexpr BYTE DrawAdvancedOptionsSearchBytes[]{ 0x55, 0x84, 0xC0, 0x0F, 0x94, 0xC2 };
    const DWORD DrawAdvancedOptionsAddr = SearchAndGetAddresses(0x00464F86, 0x00465216, 0x00465426, DrawAdvancedOptionsSearchBytes, sizeof(DrawAdvancedOptionsSearchBytes), 0x5D, __FUNCTION__);

    constexpr BYTE SkipDisabledOptionsSearchBytes[]{ 0x83, 0xC4, 0x0C, 0x85, 0xC0, 0x75, 0x19, 0x66, 0xA1 };
    const DWORD SkipDisabledOptionsAddr = SearchAndGetAddresses(0x0045FC06, 0x0045FE66, 0x0045FE66, SkipDisabledOptionsSearchBytes, sizeof(SkipDisabledOptionsSearchBytes), 0x2C, __FUNCTION__);

    constexpr BYTE InitOptionsSearchBytes[]{ 0x8B, 0x44, 0x24, 0x10, 0x8B, 0x10, 0x8B, 0x4C, 0x24, 0x04 };
    const DWORD InitOptionsAddr = SearchAndGetAddresses(0x0045BBC0, 0x0045BE20, 0x0045BE20, InitOptionsSearchBytes, sizeof(InitOptionsSearchBytes), 0x23, __FUNCTION__);

    constexpr BYTE SpkSearchBytesC[] = { 0x8B, 0x08, 0x83, 0xC4, 0x10, 0x68, 0xA4, 0x00, 0x00, 0x00, 0x68, 0x00, 0x01, 0x00, 0x00, 0x51, 0xE8 };
    void* DSpkAddrC = (void*)SearchAndGetAddresses(0x00407368, 0x00407368, 0x00407378, SpkSearchBytesC, sizeof(SpkSearchBytesC), 0x00, __FUNCTION__);

    constexpr BYTE DxConfigResolutionSearchBytes[] = { 0x8B, 0x44, 0x24, 0x24, 0x8B, 0x4C, 0x24, 0x28, 0x50, 0x51 };
    DxConfigResolution = (BYTE*)ReadSearchedAddresses(0x00407DEE, 0x00407E8E, 0x00407E9E, DxConfigResolutionSearchBytes, sizeof(DxConfigResolutionSearchBytes), 0x12, __FUNCTION__);

    constexpr BYTE CreateOptionsSearchBytes[] = { 0x8B, 0x08, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xC1, 0xC3 };
    const DWORD CreateOptionsAddr = SearchAndGetAddresses(0x005075ED, 0x0050791D, 0x0050723D, CreateOptionsSearchBytes, sizeof(CreateOptionsSearchBytes), 0x13, __FUNCTION__);

    if (!DrawAdvancedOptionsAddr || !SkipDisabledOptionsAddr || !DSpkAddrC)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }

    const DWORD CheckChangedOptionsAddr = DrawAdvancedOptionsAddr - 0x44A;

    MsgFileAddr = (DWORD*)*(DWORD*)(DrawAdvancedOptionsAddr + 0x14);
    OptionsPage = (BYTE*)*(DWORD*)(SkipDisabledOptionsAddr + 0x02);
    OptionsState = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddr - 0x3BD);
    OptionsSelectIndex = (short*)*(DWORD*)(SkipDisabledOptionsAddr - 0x0A);
    OptionsSelectDeltaIndex = (short*)*(DWORD*)(SkipDisabledOptionsAddr - 0x04);
    OptionsSaveChangesActive = (BYTE*)*(DWORD*)(CheckChangedOptionsAddr - 0x0C);
    OptionsSaveChangesIndex = (BYTE*)*(DWORD*)(CheckChangedOptionsAddr + 0x243);
    OptionCountAddr = (BYTE*)(SkipDisabledOptionsAddr - 0x18E);
    ScreenBrightnessHandlerAddr = (void*)(DrawAdvancedOptionsAddr + 0xD98);

    jmpHandleInputAddr = (void*)(DrawAdvancedOptionsAddr + 0xC19);
    jmpSkipDisabledOptionsReturnAddr1 = (void*)(SkipDisabledOptionsAddr + 0xD0);
    jmpSkipDisabledOptionsReturnAddr2 = (void*)(SkipDisabledOptionsAddr + 0x75);
    jmpCheckChangedOptionsReturnAddr = (void*)(DrawAdvancedOptionsAddr - 0x3DF);
    jmpSaveOrDiscardChangesReturnAddr1 = (void*)(CheckChangedOptionsAddr + 0x379);
    jmpSaveOrDiscardChangesReturnAddr2 = (void*)(CheckChangedOptionsAddr + 0x3DF);

    NoiseEffectOptionValue = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddr - 0x449);
    NoiseEffectOptionCommitValue = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddr - 0x443);
    AdvancedFiltersAvailable = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddr + 0xDC);
    LensFlareAvailable = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddr + 0x12D);
    DxConfigShadowsEnabled = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddr - 0x10B);
    DxConfigFogEnabled = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddr - 0xFF);
    DxConfigMotionBlurEnabled = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddr - 0xE1);
    DxConfigDepthOfFieldEnabled = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddr - 0xE6);
    DxConfigLensFlareEnabled = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddr - 0xDB);
    HandleAdvancedFiltersChange = (void (*)())(BYTE*)(DrawAdvancedOptionsAddr - 0x16F);

    // Get functions
    prepText = (char*(*)(const char* str))(((BYTE*)DSpkAddrC + 0x15) + *(int*)((BYTE*)DSpkAddrC + 0x11));
    printText = (void (*)(const char* str, int x, int y))(((BYTE*)DSpkAddrC + 0x1E) + *(int*)((BYTE*)DSpkAddrC + 0x1A));
    printTextMsg = (void (*)(const short* msg_file, short index, int x, int y))(((BYTE*)DrawAdvancedOptionsAddr) + *(int*)((BYTE*)DrawAdvancedOptionsAddr - 0x04));
    textSetColor = (void (*)(BYTE, BYTE, BYTE, BYTE))(((BYTE*)DrawAdvancedOptionsAddr + 0x0D) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0x09));
    textSetCenterAlign = (void (*)())(((BYTE*)DrawAdvancedOptionsAddr - 0x17) + *(int*)((BYTE*)DrawAdvancedOptionsAddr - 0x1B));
    textUnsetCenterAlign = (void (*)())(((BYTE*)DrawAdvancedOptionsAddr + 0x49) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0x45));
    textSetRightAlign = (void (*)())(((BYTE*)DrawAdvancedOptionsAddr + 0x44) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0x40));
    textUnsetRightAlign = (void (*)())(((BYTE*)DrawAdvancedOptionsAddr + 0x12) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0x0E));
    drawLeftSelectArrow = (void(*)(int x, int y))(((BYTE*)DrawAdvancedOptionsAddr + 0xB3A) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0xB36));
    drawRightSelectArrow = (void(*)(int x, int y))(((BYTE*)DrawAdvancedOptionsAddr + 0xC16) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0xC12));
    getTextWidth = (int(*)(int*, USHORT*))(((BYTE*)DrawAdvancedOptionsAddr + 0xAA2) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0xA9E));
    joyDown = (bool(*)(DWORD, DWORD, DWORD))(((BYTE*)DrawAdvancedOptionsAddr + 0xD67) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0xD63));
    keyDown = (bool(*)(DWORD))(((BYTE*)DrawAdvancedOptionsAddr + 0xD88) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0xD84));
    playSound = (void(*)(int, float, int))(((BYTE*)DrawAdvancedOptionsAddr + 0x11DC) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0x11D8));
    mouseDown = (bool(*)(DWORD))(((BYTE*)DrawAdvancedOptionsAddr + 0xC20) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0xC1C));
    mouseX = (int(*)())(((BYTE*)DrawAdvancedOptionsAddr + 0xC47) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0xC43));
    mouseY = (int(*)())(((BYTE*)DrawAdvancedOptionsAddr + 0xC5F) + *(int*)((BYTE*)DrawAdvancedOptionsAddr + 0xC5B));
    updateResolution = (void (*)(BYTE))(((BYTE*)DrawAdvancedOptionsAddr - 0x1E5) + *(int*)((BYTE*)DrawAdvancedOptionsAddr - 0x1E9));
    updateRenderEffects = (void (*)())(((BYTE*)DrawAdvancedOptionsAddr - 0x1DD) + *(int*)((BYTE*)DrawAdvancedOptionsAddr - 0x1E1));

    Logging::Log() << "Patching Custom Advanced Options...";

    WriteJMPtoMemory((BYTE*)DrawAdvancedOptionsAddr, *DrawAdvancedOptionsASM, 0x06);
    WriteJMPtoMemory((BYTE*)SkipDisabledOptionsAddr, *SkipDisabledOptionsASM, 0x07);
    WriteJMPtoMemory((BYTE*)CheckChangedOptionsAddr, *CheckChangedOptionsASM, 0x05);
    WriteJMPtoMemory((BYTE*)(CheckChangedOptionsAddr + 0x242), *SaveOrDiscardChangesASM, 0x05);
    WriteJMPtoMemory((BYTE*)(CreateOptionsAddr), *CreateOptions, 0x05);
    WriteCalltoMemory((BYTE*)InitOptionsAddr, *InitOptionsASM, 0x07);
    
    // Early return in options updaate handler
    UpdateMemoryAddress((void*)(DrawAdvancedOptionsAddr + 0x11DF), "\xC3\x90\x90\x90\x90", 0x05);

    // Early return in advanced filters handler
    UpdateMemoryAddress((void*)(DrawAdvancedOptionsAddr - 0x127), "\xC3\x90\x90\x90\x90", 0x05);
}
