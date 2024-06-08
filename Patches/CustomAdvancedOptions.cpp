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
#include <memory>
#include <vector>
#include <Windows.h>
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Patches\Patches.h"

namespace
{
    constexpr BYTE kAdvancedOptionsPage = 0x07;

    DWORD* MsgFileAddr = nullptr;
    BYTE* OptionsPage = nullptr;
    short* OptionsSelectIndex = nullptr;
    short* OptionsSelectDeltaIndex = nullptr;
    void* jmpHandleInputAddr = nullptr;
    void* jmpSkipDisabledOptionsReturnAddr1 = nullptr;
    void* jmpSkipDisabledOptionsReturnAddr2 = nullptr;

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

    class OptionExeText : public OptionText
    {
    public:
        OptionExeText(const char* text) : text_(text) {}

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
        Option(int index, std::unique_ptr<OptionText> name, std::unique_ptr<OptionText> description, bool enabled = true) :
            index_(index),
            name_(std::move(name)),
            description_(std::move(description)),
            enabled_(enabled)
        {
            option_y_ = index * 0x1B + 0x64;
        }

        void AddValue(std::unique_ptr<OptionText> value)
        {
            values_.push_back(std::move(value));
        }

        const std::vector<std::unique_ptr<OptionText>>& Values() const
        {
            return values_;
        }

        virtual void Draw() const
        {
            DrawBase();
            if (*OptionsSelectIndex == index_ && enabled_)
            {
                DrawHighlight();
                DrawDescription();
                DrawSelectArrows();
            }
        }

        virtual void Run() {}

        virtual void NextValue()
        {
            if (++value_index_ >= (int)values_.size())
                value_index_ = 0;
            playSound(10000, 1.0, 0);
        }

        virtual void PreviousValue()
        {
            if (--value_index_ < 0)
                value_index_ = values_.size() - 1;
            playSound(10000, 1.0, 0);
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
                {
                    SetTextColorUnselectedChanged();
                }
                values_[value_index_]->Draw(0x10E, option_y_);
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
                {
                    SetTextColorSelectedChanged();
                }
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

    private:
        int index_;
        std::unique_ptr<OptionText> name_;
        std::unique_ptr<OptionText> description_;
        std::vector<std::unique_ptr<OptionText>> values_;
        int value_index_ = 0;
        int committed_value_index_ = 0;
        bool enabled_;
        int option_y_;
    };

    class FirstOption : public Option
    {
    public:
        FirstOption(int index, bool enabled = true) : Option(
            index,
            /*name=*/std::make_unique<OptionExeText>("\\hFirst option"),
            /*description=*/std::make_unique<OptionExeText>("\\hThis is the first option"),
            /*enabled=*/enabled
        ) {}

        void Run() override
        {
            playSound(0x3A9A, 1.0, 0);
        }
    };

    class SecondOption : public Option
    {
    public:
        SecondOption(int index, bool enabled = true) : Option(
            index,
            /*name=*/std::make_unique<OptionExeText>("\\hSecond option"),
            /*description=*/std::make_unique<OptionExeText>("\\hThis is the second option"),
            /*enabled=*/enabled
        )
        {
            AddValue(std::make_unique<OptionExeText>("\\hOne"));
            AddValue(std::make_unique<OptionExeText>("\\hTwo"));
            AddValue(std::make_unique<OptionExeText>("\\hThree"));
        }
    };

    class ThirdOption : public Option
    {
    public:
        ThirdOption(int index, bool enabled = true) : Option(
            index,
            /*name=*/std::make_unique<OptionExeText>("\\hThird option"),
            /*description=*/std::make_unique<OptionExeText>("\\hThis is the third option"),
            /*enabled=*/enabled
        )
        {
            if (enabled)
            {
                AddValue(std::make_unique<OptionExeText>("\\hA"));
                AddValue(std::make_unique<OptionExeText>("\\hB"));
                AddValue(std::make_unique<OptionExeText>("\\hC"));
            }
            else
                AddValue(std::make_unique<OptionExeText>("\\hDisabled"));
        }
    };

    class FourthOption : public Option
    {
    public:
        FourthOption(int index, bool enabled = true) : Option(
            index,
            /*name=*/std::make_unique<OptionExeText>("\\hFourth option"),
            /*description=*/std::make_unique<OptionExeText>("\\hThis is the fourth option"),
            /*enabled=*/enabled
        )
        {
            AddValue(std::make_unique<OptionExeText>("\\h1.0"));
            AddValue(std::make_unique<OptionExeText>("\\h2.0"));
            AddValue(std::make_unique<OptionExeText>("\\h3.0"));
        }
    };

    std::vector<std::unique_ptr<Option>> options;

    void InitializeOptions()
    {
        int index = 0;
        options.push_back(std::make_unique<FirstOption>(index++));
        options.push_back(std::make_unique<SecondOption>(index++));
        options.push_back(std::make_unique<ThirdOption>(index++, /*enabled=*/false));
        options.push_back(std::make_unique<FourthOption>(index++));
    }

    void DrawAdvancedOptions()
    {
        for (const auto& option : options)
        {
            option->Draw();
        }

        const short select_index = *OptionsSelectIndex;
        if (select_index < 0 && select_index >= (int)options.size())
            return;

        Option* selected_option = options[select_index].get();

        // TODO: Mouse input

        // TODO: Does not work with other confirm buttons?
        if (joyDown(0, 0x10, 0) || keyDown(0x10))
            selected_option->Run();

        if (joyDown(0, 0x2000004, 0) || keyDown(0x04))
            selected_option->PreviousValue();
        else if (joyDown(0, 0x1000008, 0) || keyDown(0x08))
            selected_option->NextValue();
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
        // If all options are disabled, fall back to the first option
        *OptionsSelectIndex = 0;
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
}

void PatchCustomAdvancedOptions()
{
    constexpr BYTE DrawAdvancedOptionsSearchBytes[]{ 0x55, 0x84, 0xC0, 0x0F, 0x94, 0xC2 };
    const DWORD DrawAdvancedOptionsAddr = SearchAndGetAddresses(0x00464F86, 0x00465216, 0x00465426, DrawAdvancedOptionsSearchBytes, sizeof(DrawAdvancedOptionsSearchBytes), 0x5D, __FUNCTION__);

    constexpr BYTE SkipDisabledOptionsSearchBytes[]{ 0x83, 0xC4, 0x0C, 0x85, 0xC0, 0x75, 0x19, 0x66, 0xA1 };
    const DWORD SkipDisabledOptionsAddr = SearchAndGetAddresses(0x0045FC06, 0x0045FE66, 0x0045FE66, SkipDisabledOptionsSearchBytes, sizeof(SkipDisabledOptionsSearchBytes), 0x2C, __FUNCTION__);

    constexpr BYTE SpkSearchBytesC[] = { 0x8B, 0x08, 0x83, 0xC4, 0x10, 0x68, 0xA4, 0x00, 0x00, 0x00, 0x68, 0x00, 0x01, 0x00, 0x00, 0x51, 0xE8 };
    void* DSpkAddrC = (void*)SearchAndGetAddresses(0x00407368, 0x00407368, 0x00407378, SpkSearchBytesC, sizeof(SpkSearchBytesC), 0x00, __FUNCTION__);

    if (!DrawAdvancedOptionsAddr || !SkipDisabledOptionsAddr || !DSpkAddrC)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find pointer address!";
        return;
    }

    MsgFileAddr = (DWORD*)*(DWORD*)(DrawAdvancedOptionsAddr + 0x14);
    OptionsPage = (BYTE*)*(DWORD*)(SkipDisabledOptionsAddr + 0x02);
    OptionsSelectIndex = (short*)*(DWORD*)(SkipDisabledOptionsAddr - 0x0A);
    OptionsSelectDeltaIndex = (short*)*(DWORD*)(SkipDisabledOptionsAddr - 0x04);
    jmpSkipDisabledOptionsReturnAddr1 = (void*)(SkipDisabledOptionsAddr + 0xD0);
    jmpSkipDisabledOptionsReturnAddr2 = (void*)(SkipDisabledOptionsAddr + 0x75);
    jmpHandleInputAddr = (void*)(DrawAdvancedOptionsAddr + 0xC19);

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

    Logging::Log() << "Patching Custom Advanced Options...";

    InitializeOptions();

    WriteJMPtoMemory((BYTE*)DrawAdvancedOptionsAddr, *DrawAdvancedOptionsASM, 0x06);
    WriteJMPtoMemory((BYTE*)SkipDisabledOptionsAddr, *SkipDisabledOptionsASM, 0x07);

    const BYTE option_count = options.size() & 0xFF;
    UpdateMemoryAddress((void*)(SkipDisabledOptionsAddr - 0x18E), &option_count, 1);
}
