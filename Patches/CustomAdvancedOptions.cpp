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
#include <functional>
#include <memory>
#include <vector>
#include "Common\Utils.h"
#include "Common\Settings.h"
#include "Logging\Logging.h"
#include "Patches\Patches.h"
#include "Resolution.h"

extern char* getRenderResolutionStr();
extern char* getRenderResolutionDescriptionStr();
extern char* getResolutionDescStr();
extern char* getHealthIndicatorStr();
extern char* getHealthIndicatorDescriptionStr();

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

    char*(*shPrepText)(const char* str);
    void(*shPrintText)(const char* str, int x, int y);
    void(*shPrintTextMsg)(const short* msg_file, short index, int x, int y);
    void(*shTextSetColor)(BYTE r, BYTE g, BYTE b, BYTE a);
    void(*shTextSetCenterAlign)();
    void(*shTextUnsetCenterAlign)();
    void(*shTextSetRightAlign)();
    void(*shTextUnsetRightAlign)();
    void(*shDrawLeftSelectArrow)(int x, int y);
    void(*shDrawRightSelectArrow)(int x, int y);
    int(*shGetTextWidth)(int* len, USHORT* str);
    bool(*shJoyDown)(DWORD port, DWORD key1, DWORD key2);
    bool(*shKeyDown)(DWORD key);
    void(*shPlaySound)(int index, float volume, int pan);
    bool(*shMouseDown)(DWORD key);
    int(*shMouseX)();
    int(*shMouseY)();
    void(*shUpdateResolution)(BYTE index);
    void(*shUpdateRenderEffects)();

    void SetTextColorUnselected() { shTextSetColor(0x3F, 0x3F, 0x3F, 0x60); }
    void SetTextColorUnselectedChanged() { shTextSetColor(0x3F, 0x3F, 0x1F, 0x60); }
    void SetTextColorSelected() { shTextSetColor(0x3F, 0x3F, 0x3F, 0xFF); }
    void SetTextColorSelectedChanged() { shTextSetColor(0x3F, 0x3F, 0x1F, 0xFF); }
    void SetTextColorDisabled() { shTextSetColor(0x33, 0x33, 0x33, 0x60); }

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
            shPrintText(shPrepText(text_), x, y);
        }

        int GetTextWidth() const override
        {
            int len = 0;
            shGetTextWidth(&len, (USHORT*)(shPrepText(text_)));
            return len;
        }

    private:
        const char* text_;
    };

    class OptionExeText : public OptionText
    {
    public:
        OptionExeText(std::function<char*()> text_func) : text_func_(std::move(text_func)) {}

        void Draw(int x, int y) const override
        {
            if (text_func_ == nullptr) return;
            shPrintText(shPrepText(text_func_()), x, y);
        }

        int GetTextWidth() const override
        {
            int len = 0;
            shGetTextWidth(&len, (USHORT*)(shPrepText(text_func_())));
            return len;
        }

    private:
        std::function<char*()> text_func_;
    };

    class OptionMsgText : public OptionText
    {
    public:
        OptionMsgText(short msg_index) : msg_index_(msg_index) {}

        void Draw(int x, int y) const override
        {
            if (msg_index_ <= 0) return;
            shPrintTextMsg((const short*)*MsgFileAddr, msg_index_, x, y);
        }

        int GetTextWidth() const override
        {
            if (msg_index_ <= 0) return 0;
            int len = 0;
            USHORT* msg_ptr = (USHORT*)(*MsgFileAddr);
            USHORT* text_ptr = msg_ptr + msg_ptr[msg_index_ + 1];
            shGetTextWidth(&len, text_ptr);
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

        // Logic to run when selecting an option with no values, e.g. "Brightness Level"
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
            if (!enabled_ || values_.size() <= 1) return;
            if (++value_index_ >= (int)values_.size())
                value_index_ = 0;
            shPlaySound(10000, 1.0, 0);
        }

        virtual void PreviousValue()
        {
            if (!enabled_ || values_.size() <= 1) return;
            if (--value_index_ < 0)
                value_index_ = values_.size() - 1;
            shPlaySound(10000, 1.0, 0);
        }

        virtual void HandleMouseSelection()
        {
            if (!enabled_) return;

            const int x = shMouseX();
            const int y = shMouseY();

            if (y < index_ * 0x1B + 0x53 || y > index_ * 0x1B + 0x6E)
                return;

            if (values_.empty())
            {
                if (name_ == nullptr) return;
                const int text_half_width = name_->GetTextWidth() / 2;
                if (x > 0xFC - text_half_width && x < 0xFC + text_half_width)
                {
                    Run();
                }
            }
            else if (values_.size() > 1)
            {
                if (x >= 0xF1 && x <= 0x10A)
                {
                    PreviousValue();
                    return;
                }
                if (value_index_ < 0 || (size_t)value_index_ >= values_.size()) return;
                const int text_width = values_[value_index_]->GetTextWidth();
                if (x > text_width + 0x104 && x < text_width + 0x11F)
                {
                    NextValue();
                }
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
                shTextUnsetCenterAlign();
                shTextSetRightAlign();
                name_->Draw(0xF3, option_y_);
                shTextUnsetRightAlign();
                if (committed_value_index_ != value_index_)
                    SetTextColorUnselectedChanged();

                if (!enabled_ && value_disabled_ != nullptr)
                    value_disabled_->Draw(0x10E, option_y_);
                else
                    values_[value_index_]->Draw(0x10E, option_y_);
            }
            else
            {
                shTextSetCenterAlign();
                shTextUnsetRightAlign();
                name_->Draw(0x100, option_y_);
            }
        }

        virtual void DrawHighlight() const
        {
            if (!enabled_) return;
            SetTextColorSelected();
            if (!values_.empty())
            {
                shTextUnsetCenterAlign();
                shTextSetRightAlign();
                name_->Draw(0xF1, option_y_ - 0x02);
                shTextUnsetRightAlign();
                if (committed_value_index_ != value_index_)
                {
                    SetTextColorSelectedChanged();
                }
                values_[value_index_]->Draw(0x10C, option_y_ - 0x02);
            }
            else
            {
                shTextSetCenterAlign();
                shTextUnsetRightAlign();
                name_->Draw(0xFE, option_y_ - 0x02);
            }
        }

        virtual void DrawSelectArrows() const
        {
            if (*OptionsSaveChangesActive > 0) return;
            if (values_.empty() || value_index_ < 0 || (size_t)value_index_ >= values_.size()) return;
            const int y = index_ * 0x1B - 0x8C;
            shDrawLeftSelectArrow(-0x05, y);

            const OptionText* value_text = values_[value_index_].get();
            if (value_text == nullptr) return;
            shDrawRightSelectArrow(value_text->GetTextWidth() + 0x1E, y);
        }

        virtual void DrawDescription() const
        {
            SetTextColorSelected();
            shTextUnsetCenterAlign();
            shTextUnsetRightAlign();
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

    class ScreenBrightnessAdvancedOption : public Option
    {
    public:
        ScreenBrightnessAdvancedOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionMsgText>((short)0x02),
            /*description=*/std::make_unique<OptionMsgText>((short)0x03)
        ) {}

        void Run() override
        {
            __asm { call ScreenBrightnessHandlerAddr }
        }
    };

    class DisplayModeAdvancedOption : public Option
    {
    public:
        DisplayModeAdvancedOption(int index) : Option(
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

    class DisplayResolutionAdvancedOption : public Option
    {
    public:
        DisplayResolutionAdvancedOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionMsgText>((short)0xA8),
            /*description=*/nullptr
        ) {}

        void Init() override
        {
            if (!values_.empty()) return;

            const auto& resolution_text = GetResolutionText();
            if (IsResolutionLocked())
            {
                AddValue(std::make_unique<OptionRawText>(resolution_text[0].resStrBuf));
                description_ = std::make_unique<OptionExeText>(getResolutionDescStr);
                return;
            }
            for (const auto& text : resolution_text)
            {
                AddValue(std::make_unique<OptionRawText>(text.resStrBuf));
            }
            description_ = std::make_unique<OptionMsgText>((short)0xCA);
            value_index_ = committed_value_index_ = *DxConfigResolution;
        }

        bool Apply() override
        {
            if (!Option::Apply()) return false;
            DisplayChangeRequired = true;
            return true;
        }
    };

    Option* DisplayResolutionAdvancedOptionPtr = nullptr;

    static const std::pair<int, const char*> kRenderResolutionScaleArray[]{
        {0, "\\hx1.0"},
        {2, "\\hx1.5"},
        {1, "\\hx2.0"},
        {4, "\\hx0.5"},
        {3, "\\hx0.75"},
    };

    class RenderResolutionAdvancedOption : public Option
    {
    public:
        RenderResolutionAdvancedOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionExeText>(getRenderResolutionStr),
            /*description=*/std::make_unique<OptionExeText>(getRenderResolutionDescriptionStr)
        )
        {
            for (const auto& [ind, text] : kRenderResolutionScaleArray)
            {
                AddValue(std::make_unique<OptionRawText>(text));
            }
        }

        void Init() override
        {
            if (IsResolutionLocked())
            {
                enabled_ = false;
                return;
            }
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
            if (DisplayResolutionAdvancedOptionPtr == nullptr) return false;

            ConfigData.ScaleWindowedResolutionOption = kRenderResolutionScaleArray[value_index_].first;
            ScaleWindowedResolution = ConfigData.ScaleWindowedResolutionOption;
            SaveConfigRequired = true;

            UpdateScaleResolution();
            DisplayChangeRequired = true;
            return true;
        }
    };

    class NoiseEffectAdvancedOption : public Option
    {
    public:
        NoiseEffectAdvancedOption(int index) : Option(
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

    class ShadowsAdvancedOption : public Option
    {
    public:
        ShadowsAdvancedOption(int index) : Option(
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

    class FogAdvancedOption : public Option
    {
    public:
        FogAdvancedOption(int index) : Option(
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

    class AdvancedFiltersAdvancedOption : public Option
    {
    public:
        AdvancedFiltersAdvancedOption(int index) : Option(
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

    class LensFlareAdvancedOption : public Option
    {
    public:
        LensFlareAdvancedOption(int index) : Option(
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

    class HealthIndicatorAdvancedOption : public Option
    {
    public:
        HealthIndicatorAdvancedOption(int index) : Option(
            index,
            /*name=*/std::make_unique<OptionExeText>(getHealthIndicatorStr),
            /*description=*/std::make_unique<OptionExeText>(getHealthIndicatorDescriptionStr)
        )
        {
            AddValue(std::make_unique<OptionMsgText>((short)0xB0));  // "Off"
            AddValue(std::make_unique<OptionMsgText>((short)0xB1));  // "On"
        }

        void Init() override
        {
            value_index_ = committed_value_index_ = ConfigData.HealthIndicatorOption;
        }

        bool Apply() override
        {
            if (!Option::Apply()) return false;
            SaveConfigRequired = true;
            ConfigData.HealthIndicatorOption = value_index_;
            DisableRedCross = !ConfigData.HealthIndicatorOption;
            return true;
        }
    };

    std::vector<std::unique_ptr<Option>> options;

    void CreateOptions()
    {
        int index = 0;
        options.push_back(std::make_unique<ScreenBrightnessAdvancedOption>(index++));
        if (DisplayModeOption) options.push_back(std::make_unique<DisplayModeAdvancedOption>(index++));
        options.push_back(std::make_unique<DisplayResolutionAdvancedOption>(index++));
        DisplayResolutionAdvancedOptionPtr = options.back().get();
        options.push_back(std::make_unique<RenderResolutionAdvancedOption>(index++));
        options.push_back(std::make_unique<NoiseEffectAdvancedOption>(index++));
        options.push_back(std::make_unique<ShadowsAdvancedOption>(index++));
        options.push_back(std::make_unique<FogAdvancedOption>(index++));
        options.push_back(std::make_unique<AdvancedFiltersAdvancedOption>(index++));
        options.push_back(std::make_unique<LensFlareAdvancedOption>(index++));
        options.push_back(std::make_unique<HealthIndicatorAdvancedOption>(index++));

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
        if (shMouseDown(0x01))
        {
            selected_option->HandleMouseSelection();
            return;
        }

        // Select submenu with keyboard/controller
        if (shJoyDown(0, 0x10, 0) || shJoyDown(0, 0x200, 0) || shKeyDown(0x10))
            selected_option->Run();

        // Increment/decrement options with keyboard/controller
        if (shJoyDown(0, 0x2000004, 0) || shKeyDown(0x04))
            selected_option->PreviousValue();
        else if (shJoyDown(0, 0x1000008, 0) || shKeyDown(0x08))
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
            if (DynamicResolution && WidescreenFix)
            {
                WSFDynamicChangeWithIndex((BYTE)DisplayResolutionAdvancedOptionPtr->ValueIndex());
            }
            shUpdateResolution((BYTE)DisplayResolutionAdvancedOptionPtr->ValueIndex());
            shUpdateRenderEffects();
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
    constexpr BYTE DrawAdvancedOptionsSearchBytesA[]{ 0x55, 0x84, 0xC0, 0x0F, 0x94, 0xC2 };
    const DWORD DrawAdvancedOptionsAddrA = SearchAndGetAddresses(0x00464F86, 0x00465216, 0x00465426, DrawAdvancedOptionsSearchBytesA, sizeof(DrawAdvancedOptionsSearchBytesA), 0x5D, __FUNCTION__);

    constexpr BYTE DrawAdvancedOptionsSearchBytesB[]{ 0x83, 0xF8, 0x05, 0x0F, 0x87, 0x3B, 0x01, 0x00, 0x00 };
    const DWORD DrawAdvancedOptionsAddrB = SearchAndGetAddresses(0x004659C2, 0x00465C64, 0x00465E74, DrawAdvancedOptionsSearchBytesB, sizeof(DrawAdvancedOptionsSearchBytesB), 0x00, __FUNCTION__);

    constexpr BYTE DrawAdvancedOptionsSearchBytesC[]{ 0x84, 0xC0, 0x76, 0x04, 0xFE, 0xC8, 0xEB, 0x02, 0xB0 };
    const DWORD DrawAdvancedOptionsAddrC = SearchAndGetAddresses(0x00465F55, 0x004661F5, 0x00466405, DrawAdvancedOptionsSearchBytesC, sizeof(DrawAdvancedOptionsSearchBytesC), 0x00, __FUNCTION__);

    constexpr BYTE SkipDisabledOptionsSearchBytes[]{ 0x83, 0xC4, 0x0C, 0x85, 0xC0, 0x75, 0x19, 0x66, 0xA1 };
    const DWORD SkipDisabledOptionsAddr = SearchAndGetAddresses(0x0045FC06, 0x0045FE66, 0x0045FE66, SkipDisabledOptionsSearchBytes, sizeof(SkipDisabledOptionsSearchBytes), 0x2C, __FUNCTION__);

    constexpr BYTE InitOptionsSearchBytes[]{ 0x8B, 0x44, 0x24, 0x10, 0x8B, 0x10, 0x8B, 0x4C, 0x24, 0x04 };
    const DWORD InitOptionsAddr = SearchAndGetAddresses(0x0045BBC0, 0x0045BE20, 0x0045BE20, InitOptionsSearchBytes, sizeof(InitOptionsSearchBytes), 0x23, __FUNCTION__);

    constexpr BYTE SpkSearchBytes[] = { 0x8B, 0x08, 0x83, 0xC4, 0x10, 0x68, 0xA4, 0x00, 0x00, 0x00, 0x68, 0x00, 0x01, 0x00, 0x00, 0x51, 0xE8 };
    void* DSpkAddr = (void*)SearchAndGetAddresses(0x00407368, 0x00407368, 0x00407378, SpkSearchBytes, sizeof(SpkSearchBytes), 0x00, __FUNCTION__);

    constexpr BYTE DxConfigResolutionSearchBytes[] = { 0x8B, 0x44, 0x24, 0x24, 0x8B, 0x4C, 0x24, 0x28, 0x50, 0x51 };
    DxConfigResolution = (BYTE*)ReadSearchedAddresses(0x00407DEE, 0x00407E8E, 0x00407E9E, DxConfigResolutionSearchBytes, sizeof(DxConfigResolutionSearchBytes), 0x12, __FUNCTION__);

    constexpr BYTE CreateOptionsSearchBytes[] = { 0x8B, 0x08, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xC1, 0xC3 };
    const DWORD CreateOptionsAddr = SearchAndGetAddresses(0x005075ED, 0x0050791D, 0x0050723D, CreateOptionsSearchBytes, sizeof(CreateOptionsSearchBytes), 0x13, __FUNCTION__);

    if (!DrawAdvancedOptionsAddrA ||
        !DrawAdvancedOptionsAddrB ||
        !DrawAdvancedOptionsAddrC ||
        !SkipDisabledOptionsAddr ||
        !InitOptionsAddr ||
        !DSpkAddr ||
        !DxConfigResolution ||
        !CreateOptionsAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }

    const DWORD CheckChangedOptionsAddr = DrawAdvancedOptionsAddrA - 0x44A;

    MsgFileAddr = (DWORD*)*(DWORD*)(DrawAdvancedOptionsAddrA + 0x14);
    OptionsPage = (BYTE*)*(DWORD*)(SkipDisabledOptionsAddr + 0x02);
    OptionsState = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddrA - 0x3BD);
    OptionsSelectIndex = (short*)*(DWORD*)(SkipDisabledOptionsAddr - 0x0A);
    OptionsSelectDeltaIndex = (short*)*(DWORD*)(SkipDisabledOptionsAddr - 0x04);
    OptionsSaveChangesActive = (BYTE*)*(DWORD*)(CheckChangedOptionsAddr - 0x0C);
    OptionsSaveChangesIndex = (BYTE*)*(DWORD*)(CheckChangedOptionsAddr + 0x243);
    OptionCountAddr = (BYTE*)(SkipDisabledOptionsAddr - 0x18E);
    ScreenBrightnessHandlerAddr = (void*)(DrawAdvancedOptionsAddrB + 0x3B9);

    jmpHandleInputAddr = (void*)(DrawAdvancedOptionsAddrB + 0x23A);
    jmpSkipDisabledOptionsReturnAddr1 = (void*)(SkipDisabledOptionsAddr + 0xD0);
    jmpSkipDisabledOptionsReturnAddr2 = (void*)(SkipDisabledOptionsAddr + 0x75);
    jmpCheckChangedOptionsReturnAddr = (void*)(DrawAdvancedOptionsAddrA - 0x3DF);
    jmpSaveOrDiscardChangesReturnAddr1 = (void*)(CheckChangedOptionsAddr + 0x379);
    jmpSaveOrDiscardChangesReturnAddr2 = (void*)(CheckChangedOptionsAddr + 0x3DF);

    NoiseEffectOptionValue = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddrA - 0x449);
    NoiseEffectOptionCommitValue = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddrA - 0x443);
    AdvancedFiltersAvailable = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddrA + 0xDC);
    LensFlareAvailable = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddrA + 0x12D);
    DxConfigShadowsEnabled = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddrA - 0x10B);
    DxConfigFogEnabled = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddrA - 0xFF);
    DxConfigMotionBlurEnabled = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddrA - 0xE1);
    DxConfigDepthOfFieldEnabled = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddrA - 0xE6);
    DxConfigLensFlareEnabled = (BYTE*)*(DWORD*)(DrawAdvancedOptionsAddrA - 0xDB);
    HandleAdvancedFiltersChange = (void(*)())(BYTE*)(DrawAdvancedOptionsAddrA - 0x16F);

    shPrepText = (char*(*)(const char* str))(((BYTE*)DSpkAddr + 0x15) + *(int*)((BYTE*)DSpkAddr + 0x11));
    shPrintText = (void(*)(const char* str, int x, int y))(((BYTE*)DSpkAddr + 0x1E) + *(int*)((BYTE*)DSpkAddr + 0x1A));
    shPrintTextMsg = (void(*)(const short* msg_file, short index, int x, int y))(((BYTE*)DrawAdvancedOptionsAddrA) + *(int*)((BYTE*)DrawAdvancedOptionsAddrA - 0x04));
    shTextSetColor = (void(*)(BYTE, BYTE, BYTE, BYTE))(((BYTE*)DrawAdvancedOptionsAddrA + 0x0D) + *(int*)((BYTE*)DrawAdvancedOptionsAddrA + 0x09));
    shTextSetCenterAlign = (void(*)())(((BYTE*)DrawAdvancedOptionsAddrA - 0x17) + *(int*)((BYTE*)DrawAdvancedOptionsAddrA - 0x1B));
    shTextUnsetCenterAlign = (void(*)())(((BYTE*)DrawAdvancedOptionsAddrA + 0x49) + *(int*)((BYTE*)DrawAdvancedOptionsAddrA + 0x45));
    shTextSetRightAlign = (void(*)())(((BYTE*)DrawAdvancedOptionsAddrA + 0x44) + *(int*)((BYTE*)DrawAdvancedOptionsAddrA + 0x40));
    shTextUnsetRightAlign = (void(*)())(((BYTE*)DrawAdvancedOptionsAddrA + 0x12) + *(int*)((BYTE*)DrawAdvancedOptionsAddrA + 0x0E));
    shDrawLeftSelectArrow = (void(*)(int x, int y))(((BYTE*)DrawAdvancedOptionsAddrB + 0x15B) + *(int*)((BYTE*)DrawAdvancedOptionsAddrB + 0x157));
    shDrawRightSelectArrow = (void(*)(int x, int y))(((BYTE*)DrawAdvancedOptionsAddrB + 0x237) + *(int*)((BYTE*)DrawAdvancedOptionsAddrB + 0x233));
    shGetTextWidth = (int(*)(int*, USHORT*))(((BYTE*)DrawAdvancedOptionsAddrB + 0xC3) + *(int*)((BYTE*)DrawAdvancedOptionsAddrB + 0xBF));
    shJoyDown = (bool(*)(DWORD, DWORD, DWORD))(((BYTE*)DrawAdvancedOptionsAddrB + 0x388) + *(int*)((BYTE*)DrawAdvancedOptionsAddrB + 0x384));
    shKeyDown = (bool(*)(DWORD))(((BYTE*)DrawAdvancedOptionsAddrB + 0x3A9) + *(int*)((BYTE*)DrawAdvancedOptionsAddrB + 0x3A5));
    shPlaySound = (void(*)(int, float, int))(((BYTE*)DrawAdvancedOptionsAddrC + 0x26A) + *(int*)((BYTE*)DrawAdvancedOptionsAddrC + 0x266));
    shMouseDown = (bool(*)(DWORD))(((BYTE*)DrawAdvancedOptionsAddrB + 0x241) + *(int*)((BYTE*)DrawAdvancedOptionsAddrB + 0x23D));
    shMouseX = (int(*)())(((BYTE*)DrawAdvancedOptionsAddrB + 0x268) + *(int*)((BYTE*)DrawAdvancedOptionsAddrB + 0x264));
    shMouseY = (int(*)())(((BYTE*)DrawAdvancedOptionsAddrB + 0x280) + *(int*)((BYTE*)DrawAdvancedOptionsAddrB + 0x27C));
    shUpdateResolution = (void(*)(BYTE))(((BYTE*)DrawAdvancedOptionsAddrA - 0x1E5) + *(int*)((BYTE*)DrawAdvancedOptionsAddrA - 0x1E9));
    shUpdateRenderEffects = (void(*)())(((BYTE*)DrawAdvancedOptionsAddrA - 0x1DD) + *(int*)((BYTE*)DrawAdvancedOptionsAddrA - 0x1E1));

    Logging::Log() << "Patching Custom Advanced Options...";

    WriteJMPtoMemory((BYTE*)DrawAdvancedOptionsAddrA, *DrawAdvancedOptionsASM, 0x06);
    WriteJMPtoMemory((BYTE*)SkipDisabledOptionsAddr, *SkipDisabledOptionsASM, 0x07);
    WriteJMPtoMemory((BYTE*)CheckChangedOptionsAddr, *CheckChangedOptionsASM, 0x05);
    WriteJMPtoMemory((BYTE*)(CheckChangedOptionsAddr + 0x242), *SaveOrDiscardChangesASM, 0x05);
    WriteJMPtoMemory((BYTE*)(CreateOptionsAddr), *CreateOptions, 0x05);
    WriteCalltoMemory((BYTE*)InitOptionsAddr, *InitOptionsASM, 0x07);

    // Set early return in options update handler
    UpdateMemoryAddress((void*)(DrawAdvancedOptionsAddrC + 0x26D), "\xC3\x90\x90\x90\x90", 0x05);

    // Set early return in advanced filters handler
    UpdateMemoryAddress((void*)(DrawAdvancedOptionsAddrA - 0x127), "\xC3\x90\x90\x90\x90", 0x05);
}
