#pragma once

#include "ConfigureHardwareWindow.hpp"
#include "LoggerComponent.hpp"

#include <filesystem>

class BridgeMainComponent : public juce::Component,
                            public juce::KeyListener,
                            public Timer,
                            public ApplicationCommandTarget,
                            public MenuBarModel
{
public:
    BridgeMainComponent(std::vector<std::shared_ptr<isobus::CANHardwarePlugin>> &canDrivers);
    ~BridgeMainComponent() override;

    bool keyPressed(const juce::KeyPress &key, juce::Component *originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component *originatingComponent) override;

    void timerCallback() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    ApplicationCommandTarget *getNextCommandTarget() override;
    void getAllCommands(juce::Array<juce::CommandID> &allCommands) override;
    void getCommandInfo(juce::CommandID commandID, ApplicationCommandInfo &result) override;
    bool perform(const InvocationInfo &info) override;
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex(int, const juce::String &) override;
    void menuItemSelected(int, int) override;

private:
    enum class CommandIDs : int
    {
        NoCommand = 0, /// 0 Is an invalid command ID
        ConfigureLogging,
        ConfigureCANHardware,
        StartStop
    };

    class ModalConfigClosed
	{
	public:
		void operator()(int result) const noexcept;
		BridgeMainComponent &mParent;

	private:
	};
	friend class ModalConfigClosed;

    juce::ApplicationCommandManager mCommandManager;
    MenuBarComponent menuBar;
    LoggerComponent logger;
    Viewport loggerViewport;
    std::unique_ptr<AlertWindow> popupMenu;
    std::unique_ptr<ConfigureHardwareWindow> configureHardwareWindow;
    std::vector<std::shared_ptr<isobus::CANHardwarePlugin>> &parentCANDrivers;
    bool hasStartBeenCalled = false;
    bool autostart = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BridgeMainComponent)
};