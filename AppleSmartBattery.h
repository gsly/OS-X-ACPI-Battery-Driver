/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __AppleSmartBattery__
#define __AppleSmartBattery__


#include <IOKit/IOService.h>
#include <IOKit/pwr_mgt/IOPMPowerSource.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>

#include "AppleSmartBatteryManager.h"

#define WATTS				0
#define AMPS				1
#define ACPI_MAX			0x7FFFFFFF
#define ACPI_UNKNOWN		0xFFFFFFFF

#define BATTERY_CHARGED		0
#define BATTERY_DISCHARGING	1
#define BATTERY_CHARGING	2
#define	BATTERY_CRITICAL	4

#define BATTERY_PRESENT		0x10	// Bit 4 - _STA Method return

// Return package from _BIF

#define BIF_POWER_UNIT			0	
#define BIF_DESIGN_CAPACITY		1
#define BIF_LAST_FULL_CAPACITY	2
#define BIF_TECHNOLOGY			3
#define	BIF_DESIGN_VOLTAGE		4
#define BIF_CAPACITY_WARNING	5
#define BIF_LOW_WARNING			6
#define BIF_GRANULARITY_1		7
#define BIF_GRANULARITY_2		8
#define BIF_MODEL_NUMBER		9
#define BIF_SERIAL_NUMBER		10
#define BIF_BATTERY_TYPE		11
#define BIF_OEM					12

// Return package from _BIX

#define	BIX_REVISION			0
#define BIX_POWER_UNIT			1
#define BIX_DESIGN_CAPACITY		2
#define BIX_LAST_FULL_CAPACITY	3
#define BIX_TECHNOLOGY			4
#define BIX_DESIGN_VOLTAGE		5
#define BIX_CAPACITY_WARNING	6
#define BIX_LOW_WARNING			7
#define BIX_CYCLE_COUNT			8
#define BIX_ACCURACY			9
#define BIX_MAX_SAMPLE_TIME		10
#define BIX_MIN_SAMPLE_TIME		11
#define BIX_MAX_AVG_INTERVAL	12
#define BIX_MIN_AVG_INTERVAL	13
#define BIX_GRANULARITY_1		14
#define BIX_GRANULARITY_2		15
#define BIX_MODEL_NUMBER		16
#define BIX_SERIAL_NUMBER		17
#define BIX_BATTERY_TYPE		18
#define BIX_OEM					19

// Return package from BBIX

#define BBIX_MANUF_ACCESS		0
#define BBIX_BATTERYMODE		1
#define BBIX_ATRATETIMETOFULL	2
#define BBIX_ATRATETIMETOEMPTY	3
#define BBIX_TEMPERATURE		4
#define BBIX_VOLTAGE			5
#define BBIX_CURRENT			6
#define BBIX_AVG_CURRENT		7
#define BBIX_REL_STATE_CHARGE	8
#define BBIX_ABS_STATE_CHARGE	9
#define BBIX_REMAIN_CAPACITY	10
#define BBIX_RUNTIME_TO_EMPTY	11
#define BBIX_AVG_TIME_TO_EMPTY	12
#define BBIX_AVG_TIME_TO_FULL	13
#define BBIX_MANUF_DATE			14
#define BBIX_MANUF_DATA			15

// Return package from _BIF

#define BST_STATUS				0
#define	BST_RATE				1
#define	BST_CAPACITY			2
#define	BST_VOLTAGE				3

#define NUM_BITS				32

// Define this in Info.plist to override the default polling inverval

#define kBatteryPollingDebugKey     "BatteryPollingPeriodOverride"

// Define this in Info.plist to use extended battery information (ACPI 4.0 _BIX method)

#define kUseBatteryExtendedInfoKey	"UseExtendedBatteryInformationMethod"

// Define this in Info.plist to use extra battery information in non-standard BBIX method

#define kUseBatteryExtraInfoKey		"UseExtraBatteryInformationMethod"

static const OSSymbol * unknownObjectKey		= OSSymbol::withCString("Unknown");
UInt32 GetValueFromArray(OSArray * array, UInt8 index);
OSSymbol *GetSymbolFromArray(OSArray * array, UInt8 index);
OSData	*GetDataFromArray(OSArray *array, UInt8 index);

class AppleSmartBatteryManager;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class AppleSmartBattery : public IOPMPowerSource 
{
	OSDeclareDefaultStructors(AppleSmartBattery)

protected:

    AppleSmartBatteryManager *fProvider;
	IOWorkLoop              *fWorkLoop;
	IOTimerEventSource      *fPollTimer;
	bool					fPollingNow;
	bool					fCancelPolling;
    IOTimerEventSource      *fBatteryReadAllTimer;
    uint16_t                fMachinePath;
    uint32_t                fPollingInterval;
    bool                    fPollingOverridden;
	bool					fUseBatteryExtendedInformation;
	bool					fUseBatteryExtraInformation;
    bool                    fBatteryPresent;
    bool                    fACConnected;
	bool                    fACChargeCapable;
	
	bool					fSystemSleeping;
	IOService				*fPowerServiceToAck;
	
    OSArray                 *fCellVoltages;

	uint8_t                 fInitialPollCountdown;;
	
    // Accessor for MaxError reading
    // Percent error in MaxCapacity reading
    void    setMaxErr(int error);
    int     maxErr(void);

    // ACPIBattery reports a device name
    void    setDeviceName(OSSymbol *sym);
    OSSymbol *deviceName(void);

    // Set when battery is fully charged;
    // Clear when battery starts discharging/AC is removed
    void    setFullyCharged(bool);
    bool    fullyCharged(void);

    // Time remaining estimate - as measured instantaneously
    void    setInstantaneousTimeToEmpty(int seconds);
    
    // Time remaining until full estimate - as measured instantaneously
    void    setInstantaneousTimeToFull(int seconds);
    
    // Instantaneous amperage
    void    setInstantAmperage(int mA);

    // Time remaining estimate - 1 minute average
    void    setAverageTimeToEmpty(int seconds);
    int     averageTimeToEmpty(void);

    // Time remaining until full estimate - 1 minute average
    void    setAverageTimeToFull(int seconds);
    int     averageTimeToFull(void);
    
    void    setManufactureDate(int date);
    int     manufactureDate(void);

    void    setSerialNumber(OSSymbol *sym);
    OSSymbol *serialNumber(void);

    // An OSData container of manufacturer specific data
    void    setManufacturerData(uint8_t *buffer, uint32_t bufferSize);

	void	setChargeStatus(const OSSymbol *sym);
	const OSSymbol	*chargeStatus(void);
	
    void    oneTimeBatterySetup(void);
    
    void    constructAppleSerialNumber(void);

	void    setDesignCapacity(unsigned int val);
    unsigned int designCapacity(void);
	
	void    setBatteryType(OSSymbol *sym);
    OSSymbol *batteryType(void);
	
	void    setPermanentFailureStatus(unsigned int val);
    unsigned int permanentFailureStatus(void);
	
	void	setRunTimeToEmpty(int seconds);
	int		runTimeToEmpty(void);
	
	void	setRelativeStateOfCharge(int percent);
	int		relativeStateOfCharge(void);
	
	void	setAbsoluteStateOfCharge(int percent);
	int		absoluteStateOfCharge(void);
	
	void	setRemainingCapacity(int mah);
	int		remainingCapacity(void);
	
	void	setAverageCurrent(int ma);
	int		averageCurrent(void);
	
	void	setCurrent(int ma);
	int		current(void);
	
	void	setTemperature(int temperature);
	int		temperature(void);

	const OSSymbol	*unpackDate(UInt32 packedDate);

public:

	static AppleSmartBattery *smartBattery(void);

    virtual bool init(void);
    virtual void free(void);
	virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);

    void    setPollingInterval(int milliSeconds);

    bool    pollBatteryState(int path = 0);
    
    IOReturn setPowerState(unsigned long which, IOService *whom);

    void    handleBatteryInserted(void);
    
    void    handleBatteryRemoved(void);
	
	IOReturn handleSystemSleepWake(IOService *powerSource, bool isSystemSleep);
	
protected:
    
	void    logReadError( const char *error_type, 
                          uint16_t additional_error,
                          void *t);

    void    clearBatteryState(bool do_update);

    void    pollingTimeOut(void);
    
    void    incompleteReadTimeOut(void);

    void    rebuildLegacyIOBatteryInfo(bool do_update);

	void	acknowledgeSystemSleepWake(void);
	
private:
	
	UInt32   fPowerUnit;
	UInt32   fDesignVoltage;
	UInt32   fCurrentVoltage;
	UInt32   fDesignCapacity;
	UInt32   fCurrentCapacity;
	UInt32	 fBatteryTechnology;
	UInt32   fMaxCapacity;
	UInt32   fCurrentRate;
	UInt32   fAverageRate;
	UInt32   fStatus;
	UInt32	 fCycleCount;

	OSSymbol *fDeviceName;
	OSSymbol *fType;
	OSSymbol *fManufacturer;
	OSSymbol *fSerialNumber;

	UInt32   fMaxErr;
	UInt32   fCellVoltage1;
	UInt32   fCellVoltage2;
	UInt32   fCellVoltage3;
	UInt32   fCellVoltage4;

	UInt32	fManufacturerAccess;
	UInt32	fBatteryMode;
	UInt32	fAtRateTimeToFull;
	UInt32	fAtRateTimeToEmpty;
	UInt32  fTemperature;
	UInt32	fVoltage;
	SInt32	fCurrent;
	SInt32	fAverageCurrent;
	UInt32	fRelativeStateOfCharge;
	UInt32	fAbsoluteStateOfCharge;
	UInt32	fRemainingCapacity;
	UInt32	fRunTimeToEmpty;
	UInt32	fAverageTimeToEmpty;
	UInt32	fAverageTimeToFull;
	UInt32  fManufactureDate;
	OSData   *fManufacturerData;

public:

	IOReturn setBatterySTA(UInt32 battery_status);
	IOReturn setBatteryBIF(OSArray *acpibat_bif);
	IOReturn setBatteryBIX(OSArray *acpibat_bix);
	IOReturn setBatteryBBIX(OSArray *acpibat_bbix);
	IOReturn setBatteryBST(OSArray *acpibat_bst);

};

#endif