#include "BridgeMainComponent.hpp"

#include "AlarmMaskAudio.h"
#include "Main.hpp"
#include "ShortcutsWindow.hpp"

#ifdef JUCE_WINDOWS
#include "isobus/hardware_integration/toucan_vscp_canal.hpp"
#elif JUCE_LINUX
#include "isobus/hardware_integration/socket_can_interface.hpp"
#endif

#include <chrono>
#include <fstream>
#include <sstream>

BridgeMainComponent::BridgeMainComponent(std::vector<std::shared_ptr<isobus::CANHardwarePlugin>> &canDrivers)
    : parentCANDrivers(canDrivers)
{
    isobus::CANStackLogger::set_can_stack_logger_sink(&logger);
    isobus::CANStackLogger::set_log_level(isobus::CANStackLogger::LoggingLevel::Info);

    setSize(800, 600);

    addAndMakeVisible(menuBar);
    menuBar.setModel(this);
    
    addAndMakeVisible(loggerViewport);
    loggerViewport.setViewedComponent(&logger, false);

    logger.setVisible(true);

	mCommandManager.registerAllCommandsForTarget(this);
	setApplicationCommandManagerToWatch(&mCommandManager);
    
    setWantsKeyboardFocus(true);
    addKeyListener(this);

    startTimer(50);
}

BridgeMainComponent::~BridgeMainComponent() = default;

void BridgeMainComponent::timerCallback()
{
    // Timer logic can be added here if needed
}

void BridgeMainComponent::paint(juce::Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void BridgeMainComponent::resized()
{
    auto lMenuBarHeight = juce::LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight();
    
    menuBar.setBounds(0, 0, getWidth(), lMenuBarHeight);
	loggerViewport.setBounds(0, lMenuBarHeight, getWidth(), getHeight() - lMenuBarHeight);
}

bool BridgeMainComponent::keyPressed(const KeyPress &key, Component *originatingComponent)
{
	return false;
}

bool BridgeMainComponent::keyStateChanged(bool isKeyDown, Component *originatingComponent)
{
	return false;
}

ApplicationCommandTarget *BridgeMainComponent::getNextCommandTarget()
{
    return nullptr;
}

void BridgeMainComponent::getAllCommands(juce::Array<juce::CommandID> &allCommands)
{
    allCommands.add(static_cast<int>(CommandIDs::ConfigureLogging));
    allCommands.add(static_cast<int>(CommandIDs::ConfigureCANHardware));
    allCommands.add(static_cast<int>(CommandIDs::StartStop));
}

void BridgeMainComponent::getCommandInfo(juce::CommandID commandID, ApplicationCommandInfo &result)
{
    switch (static_cast<CommandIDs>(commandID))
    {
        case CommandIDs::ConfigureLogging:
            result.setInfo("Logging", "Change the logging level", "Configure", 0);
            break;

        case CommandIDs::ConfigureCANHardware:
            result.setInfo("Configure CAN Hardware", "Selects which CAN hardware to connect to", "Configure", 0);
            break;

        case CommandIDs::StartStop:
            result.setInfo("Start/Stop", "Starts or stops the CAN interface", "Control", hasStartBeenCalled ? ApplicationCommandInfo::CommandFlags::isTicked : 0);
            break;

        default:
            break;
    }
}

bool BridgeMainComponent::perform(const InvocationInfo &info)
{
    bool retVal = false;

	switch (info.commandID)
    {
        case static_cast<int>(CommandIDs::ConfigureLogging):
        {
			popupMenu = std::make_unique<AlertWindow>("Configure Logging", "You can use this to change the logging level, which affects what's shown in the logging area, and what is written to the log file. Setting logging to \"debug\" may impact performance, but will provide very verbose output.", MessageBoxIconType::NoIcon);
			popupMenu->addComboBox("Logging Level", { "Debug", "Info", "Warning", "Error", "Critical" });
			popupMenu->getComboBoxComponent("Logging Level")->setSelectedItemIndex(static_cast<int>(isobus::CANStackLogger::get_log_level()));
			popupMenu->addButton("OK", 4, KeyPress(KeyPress::returnKey, 0, 0));
			popupMenu->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));
			popupMenu->enterModalState(true, ModalCallbackFunction::create(ModalConfigClosed{ *this }));
			retVal = true;
		}
		break;

		case static_cast<int>(CommandIDs::ConfigureCANHardware):
		{
			configureHardwareWindow = std::make_unique<ConfigureHardwareWindow>(*this, parentCANDrivers);
			configureHardwareWindow->addToDesktop();
			juce::Rectangle<int> area(0, 0, 400, 280);
			RectanglePlacement placement(RectanglePlacement::centred | RectanglePlacement::doNotResize);
			auto result = placement.appliedTo(area, Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.reduced(20));
			configureHardwareWindow->setBounds(result);
			configureHardwareWindow->setVisible(true);
			retVal = true;
		}
		break;

		case static_cast<int>(CommandIDs::StartStop):
		{
			if (hasStartBeenCalled)
			{
				isobus::CANStackLogger::info("Stopping CAN interface");

				// Save the frame handlers so we can re-add them after stopping the interface
#ifdef JUCE_WINDOWS
				auto canDriver0 = isobus::CANHardwareInterface::get_assigned_can_channel_frame_handler(0);
#else
				auto canDriver = isobus::CANHardwareInterface::get_assigned_can_channel_frame_handler(0);
#endif

				isobus::CANHardwareInterface::stop();

				// Since "Stop" clears all frame handlers, we need to re-add the ones we saved
#ifdef JUCE_WINDOWS
				isobus::CANHardwareInterface::assign_can_channel_frame_handler(0, canDriver0);
#else
				isobus::CANHardwareInterface::assign_can_channel_frame_handler(0, canDriver);
#endif

				hasStartBeenCalled = false;
			}
			else if (nullptr == isobus::CANHardwareInterface::get_assigned_can_channel_frame_handler(0))
			{
				AlertWindow::showAsync(MessageBoxOptions()
				                         .withIconType(MessageBoxIconType::InfoIcon)
				                         .withTitle("No CAN hardware has been configured yet! Nothing to bridge, select which CAN driver to use.")
				                         .withButton("OK"),
				                       nullptr);
			}
			else
			{
				isobus::CANStackLogger::info("Starting CAN interface");
				isobus::CANHardwareInterface::start();
				hasStartBeenCalled = true;
			}
			mCommandManager.commandStatusChanged();
			retVal = true;
		}
		break;

		default:
            break;
    }
    return retVal;
}

StringArray BridgeMainComponent::getMenuBarNames()
{
    return juce::StringArray("Control");
}

PopupMenu BridgeMainComponent::getMenuForIndex(int index, const juce::String &)
{
    juce::PopupMenu retVal;

    switch (index)
    {
        case 0:
            retVal.addCommandItem(&mCommandManager, static_cast<int>(CommandIDs::ConfigureLogging));
            retVal.addCommandItem(&mCommandManager, static_cast<int>(CommandIDs::ConfigureCANHardware));
            retVal.addCommandItem(&mCommandManager, static_cast<int>(CommandIDs::StartStop));
            break;

        default:
            break;
    }
    return retVal;
}

void BridgeMainComponent::menuItemSelected(int, int)
{
    // Do nothing
}

void BridgeMainComponent::ModalConfigClosed::operator()(int result) const noexcept
{
	switch (result)
	{
		case 4: // Log level
		{
			isobus::CANStackLogger::set_log_level(static_cast<isobus::CANStackLogger::LoggingLevel>(mParent.popupMenu->getComboBoxComponent("Logging Level")->getSelectedItemIndex()));
		}
		break;

		default:
		{
			// Cancel. Do nothing
		}
		break;
	}
	mParent.exitModalState(result);
	mParent.popupMenu.reset();
}