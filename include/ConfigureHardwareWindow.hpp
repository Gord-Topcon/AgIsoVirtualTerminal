//================================================================================================
/// @file ConfigureHardwareWindow.hpp
///
/// @brief Defines a GUI window which allows selecting and configuring a hardware interface.
/// @author Adrian Del Grosso
///
/// @copyright 2023 The Open-Agriculture Contributors
//================================================================================================
#ifndef CONFIGURE_HARDWARE_WINDOW_HPP
#define CONFIGURE_HARDWARE_WINDOW_HPP

#include "ConfigureHardwareComponent.hpp"

class BridgeMainComponent;

class ConfigureHardwareWindow : public DocumentWindow
{
public:
	ConfigureHardwareWindow(BridgeMainComponent &parentComponent, std::vector<std::shared_ptr<isobus::CANHardwarePlugin>> &canDrivers);

	void closeButtonPressed() override;

	BridgeMainComponent &parentServer;

private:
	ConfigureHardwareComponent content;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigureHardwareWindow)
};

#endif // CONFIGURE_HARDWARE_WINDOW_HPP
