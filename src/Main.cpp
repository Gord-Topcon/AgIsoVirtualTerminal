/*******************************************************************************
** @file       Main.cpp
** @author     Adrian Del Grosso
** @copyright  The Open-Agriculture Developers
*******************************************************************************/
#include "isobus/hardware_integration/available_can_drivers.hpp"
#include "Main.hpp"
#include "Settings.hpp"
#include "git.h"
#include "KVCanBridge.hpp"

AgISOVirtualTerminalApplication::MainWindow::MainWindow(juce::String name) :
  DocumentWindow(name,
                 juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
                 DocumentWindow::allButtons),
  vCanBridge(std::make_unique<KVCanBridge>(0, canBITRATE_250K))
{
#ifdef JUCE_WINDOWS
	canDrivers.push_back(std::make_shared<isobus::PCANBasicWindowsPlugin>(static_cast<WORD>(PCAN_USBBUS1)));
#ifdef ISOBUS_WINDOWSINNOMAKERUSB2CAN_AVAILABLE
	canDrivers.push_back(std::make_shared<isobus::InnoMakerUSB2CANWindowsPlugin>(0));
#else
	canDrivers.push_back(nullptr);
#endif
	canDrivers.push_back(std::make_shared<isobus::TouCANPlugin>(static_cast<std::int16_t>(0), 0));
	canDrivers.push_back(std::make_shared<isobus::SysTecWindowsPlugin>());
#elif defined(JUCE_MAC)
	canDrivers.push_back(std::make_shared<isobus::MacCANPCANPlugin>(PCAN_USBBUS1));
#else
	canDrivers.push_back(std::make_shared<isobus::SocketCANInterface>("can0"));
#endif

	jassert(!canDrivers.empty()); // You need some kind of CAN interface to run this program!
	isobus::CANHardwareInterface::set_number_of_can_channels(1);

#ifndef JUCE_WINDOWS
	isobus::CANHardwareInterface::assign_can_channel_frame_handler(0, canDrivers.at(0));
#endif
	
	setUsingNativeTitleBar(true);
	setContentOwned(new BridgeMainComponent(canDrivers), true);

#if JUCE_IOS || JUCE_ANDROID
	setFullScreen(true);
#else
	setResizable(true, true);
	centreWithSize(getWidth(), getHeight());
#endif

	setIcon(ImageCache::getFromMemory(AppImages::logosmall_png, AppImages::logosmall_pngSize));
#if JUCE_LINUX
	// this hack is needed on Linux
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(ImageCache::getFromMemory(AppImages::logosmall_png, AppImages::logosmall_pngSize));
	}
#endif
	setVisible(true);
}

void AgISOVirtualTerminalApplication::MainWindow::closeButtonPressed()
{
	// This is called when the user tries to close this window. Here, we'll just
	// ask the app to quit when this happens, but you can change this to do
	// whatever you need.
	isobus::CANHardwareInterface::stop();
	JUCEApplication::getInstance()->systemRequestedQuit();
}

std::string AgISOVirtualTerminalApplication::getApplicationBuildInfo()
{
	std::string gitDescribe = std::string(git::Describe());
	if (gitDescribe.length() > 0)
	{
		return gitDescribe + (git::AnyUncommittedChanges() ? "-dirty" : "");
	}
	return ProjectInfo::versionString;
}

std::string AgISOVirtualTerminalApplication::getApplicationNameWithBuildInfo()
{
	std::string name = ProjectInfo::projectName;
	auto buildInfo = getApplicationBuildInfo();
	if (buildInfo.length() > 0)
	{
		name.append(" - " + buildInfo);
	}
	return name;
}

START_JUCE_APPLICATION(AgISOVirtualTerminalApplication)
