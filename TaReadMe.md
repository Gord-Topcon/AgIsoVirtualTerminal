# Topcon Specific ReadMe

## About

Topcon modifications were made to this project to generate tools for use on Windows and elsewhere.
Topcon code is currently housed in a fork of the upstream code located at : https://github.com/Gord-Topcon/AgIsoVirtualTerminal

## Branches

### main 
- not necessarily up to date but unmodified code from upstream repo
	
### develop (UTSim)
- created from main at the time the fork was created
- has an additional driver for connecting to a Kvaser virtual bus (1)
- if/when we want to include changes made upstream rebase this branch on (updated) main branch

###	experiment/SCYM-153_KvasserVBus_Bridge (PBridgeV)
- modified application to bridge selected hardware connection to a Kvaser virtual bus (0)
	
## Development

In general follow the main README for guidance on developing.

Additional notes :

- CMakePresets.json - (added) at the repo root is a setup file VScode can use to provide build targets on Windows
- Topcan additions require the installation of the Kvaser SDK and driver set available from https://www.kvaser.com/download/
- Once installed the following file is particularily helpful "C:\Program Files (x86)\Kvaser\Canlib\Doc\canlib.chm"

## Usage

### Windows

To make use of additional features requires the installation of the Kvaser drivers (only)

Two applications can be built. (One from the experimental branch listed above)

A successful build will result in artifacts being created :

<repo root>\out\build\VSamd64\AgISOVirtualTerminal_artefacts\Debug
- Usbcan64.dll
- AgISOVirtualTerminal.exe
- AgISOVirtualTerminal.pdb
- canal.dll
- canlib32.dll
- InnoMakerUsb2CanLib.dll
- PCANBasic.dll

Copy the contents to a (separate) folder on the target machine and run from there.

**NOTE:** Pre-built archives are at the repo root PBridgeV.7z and UTSim.7z

Settings and data for the applications will be stored at the following path

	C:\Users\<Username>\AppData\Roaming\<Application Name>

Depending on the application this folder may include

- AgISOVirtualTerminalLog.txt - debug log
- CANLog_<Timestamp>.asc - CAN bus log
- vt_settings.xml - definitions for the type of UT simulated
- \iso_data - a folder containing collected object pool information


#### Typical Workflow

**Note:** if a hardware(PCAN etc.) connection isn't required skip to step 4

1. Start bridge application
2. Select hardware connection from the drop down list in Control/Configure CAN Hardware/ (Typically PEAK PCAN USB)
3. Select Control/Start
4. Start the standard application
5. Select bus connection from the drop down list in Control/Configure CAN Hardware/ (Typically Kvaser VCan1)
6. Select Control/Start

##### Things to Note

- feedback can be limited.  Sometimes it is working but there is no indication that it is.  Waiting may help
- It doesn't always seem to "reconnect" to a running UT that it was previously connected to.  Sometimes you need to restart the UT.

