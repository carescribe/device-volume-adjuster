//
//  adjust_volume.c
//  AdjustVolume
//
//  Created by Jono Muller on 29/12/2017.
//  Copyright © 2017 Jonathan Muller. All rights reserved.
//

#include "adjust_volume.h"

int startVolumeAdjuster(int argc, const char * argv[]) {
  int opt;
  int functionType = 0;
  char deviceName[256];
  Float32 newVolume = -1.0;
  
  while ((opt = getopt(argc, (char **) argv, "n:ids:")) != -1) {
    switch (opt) {
      case 'n':
        strcpy(deviceName, optarg);
        break;
      case 'i':
        functionType = kIncrementVolume;
        break;
      case 'd':
        functionType = kDecrementVolume;
        break;
      case 's':
        functionType = kSetVolume;
        newVolume = atof(optarg);
        break;
      default:
        break;
    }
  }
  
  if (strlen(deviceName) == 0) {
    printf("-n is required, please enter a device name.");
    return 1;
  }
  
  if (functionType == 0) {
    printf("No function selected. Please select one function to adjust the volume (-i, -d, -s).");
    return 1;
  }
  
  AudioDeviceID deviceID = getDeviceID(deviceName);
  AudioObjectPropertyAddress propertyAddress =  {
    kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
    kAudioDevicePropertyScopeOutput,
    kAudioObjectPropertyElementMaster
  };
  
  if (functionType != kSetVolume) {
    newVolume = getNewVolume(deviceID, functionType, propertyAddress);
  }
  
  // Check for newVolume < 0 ?
  setVolume(deviceID, newVolume, propertyAddress);
  return 0;
}

AudioDeviceID getDeviceID(char * deviceName) {
  UInt32 propertySize;
  int numberOfDevices = 0;
  AudioObjectID defaultID = kAudioObjectSystemObject;
  
  // Get address for all hardware devices
  AudioObjectPropertyAddress devicesAddress = {
    kAudioHardwarePropertyDevices,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster };
  
  // Retreive property size within devices address
  AudioObjectGetPropertyDataSize(defaultID, &devicesAddress, 0, nil, &propertySize);
  
  numberOfDevices = propertySize / sizeof(AudioObjectID);
  AudioDeviceID ids[numberOfDevices];
  
  // Populate ids from devices address
  AudioObjectGetPropertyData(defaultID, &devicesAddress, 0, nil, &propertySize, ids);
  
  for (int i = 0; i < numberOfDevices; i++) {
    UInt32 nameSize = 256;
    char name[nameSize];
    
    AudioObjectPropertyAddress deviceNameAddress = {
      kAudioDevicePropertyDeviceName,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster };
    
    // Get device name from id
    AudioObjectGetPropertyData(ids[i], &deviceNameAddress, 0, nil, &nameSize, (char *) name);
    
    // Check if names match
    if (strcmp(deviceName, (char *) name) == 0) {
      return ids[i];
    }
  }
  
  return kAudioDeviceUnknown;
}

Float32 getNewVolume(AudioDeviceID deviceID, FunctionType type, AudioObjectPropertyAddress propertyAddress) {
  Float32 volume;
  UInt32 volumeSize = sizeof(volume);
  Float32 incLevel = 1.0 / 16.0;
  
  // Get current volume level for requested device
  AudioObjectGetPropertyData(deviceID, &propertyAddress, 0, nil, &volumeSize, &volume);
  
  switch (type) {
    case kIncrementVolume:
      return volume + incLevel;
      break;
    case kDecrementVolume:
      return volume - incLevel;
    default:
      return -1.0;
      break;
  }
}

void setVolume(AudioDeviceID deviceID, Float32 volume, AudioObjectPropertyAddress propertyAddress) {
  UInt32 volumeSize = sizeof(volume);
  
  // Set new volume level
  AudioObjectSetPropertyData(deviceID, &propertyAddress, 0, nil, volumeSize, &volume);
}
