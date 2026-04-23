#pragma once

#include "../imgui.hpp"
#include "../imgui_internal.hpp"
#include "../../../../Cheat/Menu/Font/FontAwesome.hpp"
#include "../../../../Cheat/Menu/Font/Byte.hpp"

constexpr int NOTIFY_MAX_MSG_LENGTH = 4096;
constexpr float NOTIFY_PADDING_X = 20.f;
constexpr float NOTIFY_PADDING_Y = 20.f;
constexpr float NOTIFY_PADDING_MESSAGE_Y = 10.f;
constexpr int NOTIFY_FADE_IN_OUT_TIME = 150;
constexpr int NOTIFY_DEFAULT_DISMISS = 3000;
constexpr float NOTIFY_OPACITY = 1.f;
constexpr bool NOTIFY_USE_SEPARATOR = false;
constexpr bool NOTIFY_USE_DISMISS_BUTTON = false;
constexpr int NOTIFY_RENDER_LIMIT = 5;
constexpr bool NOTIFY_RENDER_OUTSIDE_MAIN_WINDOW = true;

static const ImGuiWindowFlags NOTIFY_DEFAULT_TOAST_FLAGS =
ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration |
ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus |
ImGuiWindowFlags_NoFocusOnAppearing;

#define NOTIFY_NULL_OR_EMPTY(str) (!str || !strlen(str))
#define NOTIFY_FORMAT(fn, format, ...) if (format) { \
    va_list args; va_start(args, format); fn(format, args, ##__VA_ARGS__); va_end(args); \
}

enum class ImGuiToastType : uint8_t {
    None, Success, Warning, Error, Info, COUNT
};

enum class ImGuiToastPhase : uint8_t {
    FadeIn, Wait, FadeOut, Expired, COUNT
};

enum class ImGuiToastPos : uint8_t {
    TopLeft, TopCenter, TopRight, BottomLeft, BottomCenter, BottomRight, Center, COUNT
};

class ImGuiToast {
private:
    ImGuiWindowFlags flags = NOTIFY_DEFAULT_TOAST_FLAGS;
    ImGuiToastType type = ImGuiToastType::None;
    std::string title;
    std::string content;
    int dismissTime = NOTIFY_DEFAULT_DISMISS;
    std::chrono::system_clock::time_point creationTime = std::chrono::system_clock::now();
    std::function<void()> onButtonPress = nullptr;
    std::string buttonLabel;

    // Función común para setear contenido de texto
    void setText(std::string& target, const char* format, va_list args) {
        char buffer[NOTIFY_MAX_MSG_LENGTH];
        vsnprintf(buffer, sizeof(buffer), format, args);
        target = buffer;
    }

public:
    ImGuiToast(ImGuiToastType type, int dismissTime = NOTIFY_DEFAULT_DISMISS)
        : type(type), dismissTime(dismissTime) {}

    ImGuiToast(ImGuiToastType type, const char* format, ...)
        : ImGuiToast(type) {
        NOTIFY_FORMAT([this](const char* fmt, va_list args) {
            setText(content, fmt, args);
        }, format);
    }

    ImGuiToast(ImGuiToastType type, int dismissTime, const char* format, ...)
        : ImGuiToast(type, dismissTime) {
        NOTIFY_FORMAT([this](const char* fmt, va_list args) {
            setText(content, fmt, args);
        }, format);
    }

    ImGuiToast(ImGuiToastType type, int dismissTime, const char* buttonLabel,
        const std::function<void()>& onButtonPress, const char* format, ...)
        : ImGuiToast(type, dismissTime) {
        NOTIFY_FORMAT([this](const char* fmt, va_list args) {
            setText(content, fmt, args);
        }, format);
        this->onButtonPress = onButtonPress;
        this->buttonLabel = buttonLabel;
    }

    void setTitle(const char* format, ...) {
        NOTIFY_FORMAT([this](const char* fmt, va_list args) {
            setText(title, fmt, args);
        }, format);
    }

    void setContent(const char* format, ...) {
        NOTIFY_FORMAT([this](const char* fmt, va_list args) {
            setText(content, fmt, args);
        }, format);
    }

    void setType(const ImGuiToastType& type) { this->type = type; }
    void setWindowFlags(const ImGuiWindowFlags& flags) { this->flags = flags; }
    void setOnButtonPress(std::function<void()>&& onButtonPress) {
        this->onButtonPress = std::move(onButtonPress);
    }
    void setButtonLabel(const char* label) { buttonLabel = label; }

    const char* getTitle() const { return title.c_str(); }
    const char* getDefaultTitle() const {
        if (title.empty()) {
            switch (type) {
            case ImGuiToastType::Success: return "Success";
            case ImGuiToastType::Warning: return "Warning";
            case ImGuiToastType::Error: return "Error";
            case ImGuiToastType::Info: return "Info";
            default: return nullptr;
            }
        }
        return title.c_str();
    }

    ImGuiToastType getType() const { return type; }
    ImVec4 getColor() const {
        switch (type) {
        case ImGuiToastType::Success: return { 0, 255, 0, 255 };
        case ImGuiToastType::Warning: return { 255, 255, 0, 255 };
        case ImGuiToastType::Error: return { 255, 0, 0, 255 };
        case ImGuiToastType::Info: return { 0, 157, 255, 255 };
        default: return { 255, 255, 255, 255 };
        }
    }

    const char* getIcon() const {
        switch (type) {
        case ImGuiToastType::Success: return ICON_FA_CIRCLE_CHECK;
        case ImGuiToastType::Warning: return ICON_FA_TRIANGLE_EXCLAMATION;
        case ImGuiToastType::Error: return ICON_FA_CIRCLE_EXCLAMATION;
        case ImGuiToastType::Info: return ICON_FA_CIRCLE_INFO;
        default: return nullptr;
        }
    }

    const char* getContent() const { return content.c_str(); }
    std::chrono::nanoseconds getElapsedTime() const {
        return std::chrono::system_clock::now() - creationTime;
    }

    ImGuiToastPhase getPhase() const {
        const int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(getElapsedTime()).count();
        if (elapsed > NOTIFY_FADE_IN_OUT_TIME + dismissTime + NOTIFY_FADE_IN_OUT_TIME)
            return ImGuiToastPhase::Expired;
        if (elapsed > NOTIFY_FADE_IN_OUT_TIME + dismissTime)
            return ImGuiToastPhase::FadeOut;
        if (elapsed > NOTIFY_FADE_IN_OUT_TIME)
            return ImGuiToastPhase::Wait;
        return ImGuiToastPhase::FadeIn;
    }

    float getFadePercent() const {
        const ImGuiToastPhase phase = getPhase();
        const int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(getElapsedTime()).count();
        if (phase == ImGuiToastPhase::FadeIn)
            return ((float)elapsed / (float)NOTIFY_FADE_IN_OUT_TIME) * NOTIFY_OPACITY;
        if (phase == ImGuiToastPhase::FadeOut)
            return (1.f - (((float)elapsed - (float)NOTIFY_FADE_IN_OUT_TIME - (float)dismissTime) / (float)NOTIFY_FADE_IN_OUT_TIME)) * NOTIFY_OPACITY;
        return 1.f * NOTIFY_OPACITY;
    }

    ImGuiWindowFlags getWindowFlags() const { return flags; }
    const std::function<void()>& getOnButtonPress() const { return onButtonPress; }
    const char* getButtonLabel() const { return buttonLabel.c_str(); }
};

namespace ImGui {
    inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
        return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
    }

    inline void RenderCustomDraw() {
        auto drawList = ImGui::GetForegroundDrawList();
        auto windowPos = ImGui::GetWindowPos();
        auto windowSize = ImGui::GetWindowSize();

        drawList->AddRectFilledMultiColor(
            windowPos + ImVec2(windowSize.x / 2, 0),
            windowPos + ImVec2(windowSize.x / 2 + 92, windowSize.y),
            ImColor(139, 119, 185, 40),
            ImColor(0, 0, 0, 0),
            ImColor(0, 0, 0, 0),
            ImColor(139, 119, 185, 40)
        );

        drawList->AddRectFilledMultiColor(
            windowPos + ImVec2(0, 0),
            windowPos + ImVec2(windowSize.x / 2, windowSize.y),
            ImColor(0, 0, 0, 0),
            ImColor(139, 119, 185, 40),
            ImColor(139, 119, 185, 40),
            ImColor(0, 0, 0, 0)
        );
    }

    inline std::vector<ImGuiToast> notifications;

    inline void InsertNotification(const ImGuiToast& toast) {
        notifications.push_back(toast);
    }

    inline void RemoveNotification(int index) {
        notifications.erase(notifications.begin() + index);
    }

    inline void RenderNotifications() {
        ImGui::PushFont(Fonts::FontAwesome);
        const ImVec2 mainWindowSize = GetMainViewport()->Size;
        float height = 0.f;

        for (size_t i = 0; i < notifications.size(); ++i) {
            ImGuiToast& currentToast = notifications[i];

            if (currentToast.getPhase() == ImGuiToastPhase::Expired) {
                RemoveNotification(i);
                --i;
                continue;
            }

            #if NOTIFY_RENDER_LIMIT > 0
            if (i >= NOTIFY_RENDER_LIMIT) break;
            #endif

            const char* icon = currentToast.getIcon();
            const char* title = currentToast.getTitle();
            const char* content = currentToast.getContent();
            const char* defaultTitle = currentToast.getDefaultTitle();
            const float opacity = currentToast.getFadePercent();

            ImVec4 textColor = currentToast.getColor();
            textColor.w = opacity;

            char windowName[50];
            #ifdef _WIN32
            sprintf_s(windowName, "##TOAST%d", (int)i);
            #else
            std::snprintf(windowName, sizeof(windowName), "##TOAST%d", (int)i);
            #endif

            SetNextWindowBgAlpha(opacity);

            #if NOTIFY_RENDER_OUTSIDE_MAIN_WINDOW
            ImVec2 WorkPos = ImGui::GetMainViewport()->WorkPos;
            ImVec2 WorkSize = ImGui::GetMainViewport()->WorkSize;
            SetNextWindowPos(ImVec2(WorkPos.x + WorkSize.x - NOTIFY_PADDING_X, WorkPos.y + WorkSize.y - NOTIFY_PADDING_Y - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
            #else
            ImVec2 mainWindowPos = GetMainViewport()->Pos;
            SetNextWindowPos(ImVec2(mainWindowPos.x + mainWindowSize.x - NOTIFY_PADDING_X, mainWindowPos.y + mainWindowSize.y - NOTIFY_PADDING_Y - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
            #endif

            if (!NOTIFY_USE_DISMISS_BUTTON && !currentToast.getOnButtonPress()) {
                currentToast.setWindowFlags(NOTIFY_DEFAULT_TOAST_FLAGS | ImGuiWindowFlags_NoInputs);
            }

            Begin(windowName, nullptr, currentToast.getWindowFlags());
            RenderCustomDraw();
            BringWindowToDisplayFront(GetCurrentWindow());

            PushTextWrapPos(mainWindowSize.x / 3.f);

            bool wasTitleRendered = false;

            if (!NOTIFY_NULL_OR_EMPTY(icon)) {
                ImGui::SetCursorPos(ImVec2(15, 10));
                TextColored(textColor, "%s", icon);
                wasTitleRendered = true;
            }

            if (!NOTIFY_NULL_OR_EMPTY(title)) {
                if (wasTitleRendered) SameLine();
                Text("%s", title);
                wasTitleRendered = true;
            } else if (!NOTIFY_NULL_OR_EMPTY(defaultTitle)) {
                if (wasTitleRendered) SameLine();
                ImGui::SetCursorPos(ImVec2(35, 9));
                Text("%s", defaultTitle);
                wasTitleRendered = true;
            }

            if (NOTIFY_USE_DISMISS_BUTTON) {
                if (wasTitleRendered || !NOTIFY_NULL_OR_EMPTY(content)) SameLine();
                if (Button(ICON_FA_XMARK)) RemoveNotification(i);
            }

            if (wasTitleRendered && !NOTIFY_NULL_OR_EMPTY(content)) {
                SetCursorPosY(GetCursorPosY() + 5.f);
            }

            if (!NOTIFY_NULL_OR_EMPTY(content)) {
                if (wasTitleRendered) {
                    #if NOTIFY_USE_SEPARATOR
                    Separator();
                    #endif
                }
                Text("%s", content);
            }

            if (currentToast.getOnButtonPress()) {
                if (Button(currentToast.getButtonLabel())) {
                    currentToast.getOnButtonPress()();
                }
            }

            PopTextWrapPos();
            height += GetWindowHeight() + NOTIFY_PADDING_MESSAGE_Y;
            End();
        }
        PopFont();
    }
}