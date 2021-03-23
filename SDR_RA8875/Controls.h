//
//    Controls.h
//
//    Core functions that control things.  The controller may be a touch event from a button or label object
//      or from a mechanical device or another function in a chain of interdepedencies.
//
//    Many controls have multipe possible controllers so have to adopt an Controller->control->display model
//      allowing parallel control requests.  
//      The control object changes (or requests to change) states, the display only scans and reports the state.
//      It becomes importan to pass through this as remote control and monitoring get built.

// Using the SV1AFN Band Pass Filter board with modified I2C library for Premp, Attenuator, and for 10 HF bands of BPFs
//#include "SVN1AFN_BandpassFilters.h>""
#ifdef SV1AFN_BPF
  extern SVN1AFN_BandpassFilters bpf;
#endif

extern uint8_t curr_band;   // global tracks our current band setting.  
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t VFOB;
extern struct Band_Memory bandmem[];
extern struct User_Settings user_settings[];
extern struct Bandwidth_Settings bw[];
extern uint8_t user_Profile;
extern Metro popup_timer; // used to check for popup screen request
extern AudioControlSGTL5000 codec1;
extern uint8_t popup;
extern void RampVolume(float vol, int16_t rampType);

void Set_Spectrum_Scale(int8_t zoom_dir);
void Set_Spectrum_RefLvl(int8_t zoom_dir);
void Gesture_Handler(uint8_t gesture);
void changeBands(int8_t direction);
void pop_win(uint8_t init);
void Mute();
void Menu();
void Display();
void Band();
void BandDn();
void BandUp();
void Notch();
void Spot();
void Enet();
void NR();
void NB();
void Xmit();
void Ant();
void Fine();
void Rate(int8_t direction);
void setMode(int8_t dir);
void AGC();
void Filter();
void ATU();
void Xvtr();
void Split();
void XIT();
void RIT();
void Preamp(int8_t toggle);
void Atten(int8_t toggle);
void VFO_AB();
void setAtten(uint8_t atten);

// Use gestures (pinch) to adjust the the vertical scaling.  This affects both watefall and spectrum.  YMMV :-)
void Set_Spectrum_Scale(int8_t zoom_dir)
{
    Serial.println(zoom_dir);
    //extern struct Spectrum_Parms Sp_Parms_Def[];    
    if (Sp_Parms_Def[spectrum_preset].spect_wf_scale > 2.0) 
        Sp_Parms_Def[spectrum_preset].spect_wf_scale = 0.5;
    if (Sp_Parms_Def[spectrum_preset].spect_wf_scale < 0.5)
        Sp_Parms_Def[spectrum_preset].spect_wf_scale = 2.0; 
    if (zoom_dir == 1)
    {
        Sp_Parms_Def[spectrum_preset].spect_wf_scale += 0.1;
        Serial.println("ZOOM IN");
    }
    else
    {        
        Sp_Parms_Def[spectrum_preset].spect_wf_scale -= 0.1;
        Serial.println("ZOOM OUT"); 
    }
    Serial.println(Sp_Parms_Def[spectrum_preset].spect_wf_scale);
}

// Use gestures to raise and lower the spectrum reference level relative to the bottom of the window (noise floor)
void Set_Spectrum_RefLvl(int8_t zoom_dir)
{
    Serial.println(zoom_dir);    
    
    if (zoom_dir == 1)
    {
        Sp_Parms_Def[spectrum_preset].spect_floor -= 10;
        Serial.print("RefLvl=UP");
    }        
    else
    {
        Sp_Parms_Def[spectrum_preset].spect_floor += 10;
        Serial.print("RefLvl=DOWN");
    }
    if (Sp_Parms_Def[spectrum_preset].spect_floor < -400)
        Sp_Parms_Def[spectrum_preset].spect_floor = -400; 
    if (Sp_Parms_Def[spectrum_preset].spect_floor > 400)
        Sp_Parms_Def[spectrum_preset].spect_floor = 400;
}
//
//----------------------------------- Skip to Ham Bands only ---------------------------------
//
// Increment band up or down from present.   To be used with touch or physical band UP/DN buttons.
// A alternate method (not in this function) is to use a band button or gesture to do a pop up selection map.  
// A rotary encoder can cycle through the choices and push to select or just touch the desired band.
//
//
// --------------------- Change bands using database -----------------------------------
// Returns 0 if cannot change bands
// Returns 1 if success

void changeBands(int8_t direction)  // neg value is down.  Can jump multiple bandswith value > 1.
{
    
    // TODO search bands column for match toaccount for mapping that does not start with 0 and bands could be in odd order and disabled.
    //Serial.print("\nCurrent Band is "); Serial.println(bandmem[curr_band].band_name);
    bandmem[curr_band].vfo_A_last = VFOA;
    bandmem[curr_band].vfo_B_last = VFOB;

    // Deal with transverters later probably increase BANDS count to cover all transverter bands to (if enabled).
    int8_t target_band = bandmem[curr_band + direction].band_num;
    
    Serial.print("Target Band is "); Serial.println(target_band);

    if (target_band > BAND9)    // go to bottom band
        target_band = BAND9;    // 0 is not used
    if (target_band < BAND0)    // go to top most band  -  
        target_band = BAND0;    // 0 is not used so do not have to adjsut with a -1 here

    Serial.print("Corrected Target Band is "); Serial.println(target_band);    
  
//TODO check if band is active and if not, skip down to next until we find one active in the bandmap    
    RampVolume(0.0f, 2);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    curr_band = target_band;    // Set out new band
    VFOA = bandmem[curr_band].vfo_A_last;  // up the last used frequencies
    VFOB = bandmem[curr_band].vfo_B_last;
    Serial.print("New Band is "); Serial.println(bandmem[curr_band].band_name);     
    delay(20);  // small delay for audio ramp to work
    selectFrequency(0);  // change band and preselector
    selectBandwidth(bandmem[curr_band].filter);
    Atten(-1);  // -1 sets to database state. 2 is toggle state. 0 and 1 are Off and On.  Operate relays if any.
     //dB level is set elsewhere and uses value in the dB in this function.
    Preamp(-1);  // -1 sets to database state. 2 is toggle state. 0 and 1 are Off and On.  Operate relays if any.
    //selectMode(0);  
    setMode(0);// 0 is set value in database for both VFOs
    //Rate(0); Not needed
    //Ant() when there is hardware to setup in the future
    //ATU() when there is hardware to setup in the future
    //
    //   insert any future features, software or hardware, that need to be altered      
    //
    selectAgc(bandmem[curr_band].agc_mode);
    displayRefresh();
    RampVolume(1.0f, 2);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp" 
}

void pop_win(uint8_t init)
{
    if(init)
    {
        popup_timer.interval(300);
        tft.setActiveWindow(200, 600, 160, 360);
        tft.fillRoundRect(200,160, 400, 200, 20, RA8875_LIGHT_GREY);
        tft.drawRoundRect(200,160, 400, 200, 20, RA8875_RED);
        tft.setTextColor(RA8875_BLUE);
        tft.setCursor(CENTER, CENTER, true);
        tft.print("this is a future keyboard");
        delay(1000);
        tft.fillRoundRect(200,160, 400, 200, 20, RA8875_LIGHT_ORANGE);
        tft.drawRoundRect(200,160, 400, 200, 20, RA8875_RED);
        tft.setCursor(CENTER, CENTER, true);
        tft.print("Thanks for watching, GoodBye!");
        delay(600);
        popup = 0;
   // }
   // else 
   // {
        tft.fillRoundRect(200,160, 400, 290, 20, RA8875_BLACK);
        tft.setActiveWindow();
        popup = 0;   // resume our normal schedule broadcast
        popup_timer.interval(65000);
        drawSpectrumFrame(user_settings[user_Profile].sp_preset);
        displayRefresh();
    }
}

//
//  -----------------------   Button Functions --------------------------------------------
//   Called by Touch, Encoder, or Switch events


// ---------------------------Mode() ----------------------------------
//   Input: 0 = set to current value in the database (circular rotation through all modes)
//          1 = increment the mode
//         -1 = decrement the mode
    
// MODE button
void setMode(int8_t dir)
{
	uint8_t mndx;

	if (bandmem[curr_band].VFO_AB_Active == VFO_A)  // get Active VFO mode
		mndx = bandmem[curr_band].mode_A;			
	else
		mndx = bandmem[curr_band].mode_B;
	
	mndx += dir; // Make the change

  	if (mndx > DATA)		// Validate change and fix if needed
   		mndx=0;
	if (mndx < CW)
		mndx = CW;

    selectMode(mndx);   // Select the mode for the Active VFO 
    
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)   // Store our mode for the active VFO
		bandmem[curr_band].mode_A = mndx;
	else	
		bandmem[curr_band].mode_B = mndx;    
    
    Serial.print("Set Mode");  
    displayMode();
}

// ---------------------------Filter() ---------------------------
//   Input: 0 = step to next based on last direction (starts upwards).  Ramps up then down then up.
//          1 = step up 1 filter (wider)
//         -1 = step down 1 filter (narrower)
//      This is mode-aware. In non-CW modes we will only cycle through SSB width filters as set in the filter tables

// FILTER button
void Filter(int dir)
{ 
    static int direction = -1;
    int _bndx = bandmem[curr_band].filter; // Change Bandwidth  - cycle down then back to the top
    
    uint8_t _mode;

    // 1. Limit to allowed step range
    // 2. Cycle up and at top, cycle back down, do nto roll over.
    if (_bndx <= 0)
    {
        _bndx = -1;
        direction = 1;   // cycle upwards
    }

    if (_bndx >= FILTER-1)
    {
        _bndx = FILTER-1;
        direction = -1;
    }

    if (bandmem[curr_band].VFO_AB_Active == VFO_A)
        _mode = bandmem[curr_band].mode_A;
    else
        _mode = bandmem[curr_band].mode_B;

    if (_mode == CW)  // CW modes
    {
        if (_bndx > FILTER-1)   // go to bottom band   
        {
            _bndx = FILTER-1;
            direction = -1;     // cycle downwards
        }
        if (_bndx <=BW0_25)
        {
            _bndx = BW0_25;          
            direction = 1;      // cycle upwards
        }
    }
    else  // Non-CW modes
    {
        if (_bndx > FILTER-1)   // go to bottom band  
        { 
            _bndx = FILTER-1;    
            direction = -1;     // cycle downwards
        }
        if (_bndx <= BW1_8)
        {
            _bndx = BW1_8;
            direction = 1;      // cycle upwards
        }
    }

        if (dir == 0)
            _bndx += direction; // Index our step up or down
		else	
			_bndx += dir;  // forces a step higher or lower then current
		
    selectBandwidth(_bndx);
    Serial.print("Set Filter to ");
    Serial.println(bandmem[curr_band].filter);
    displayFilter();
}

// ---------------------------Rate() ---------------------------
//   Input: 0 = step to next based on last direction (starts upwards).  Ramps up then down then up.
//          1 = step up 1 tune rate
//         -1 = step down 1 tune step rate
//      If FINE is OFF, we will not use 1Hz. If FINE = ON we only use 1 and 10Hz steps.

// RATE button
void Rate(int8_t swiped)
{
    static int direction = 1;
	int _fndx = bandmem[curr_band].tune_step;

	if (user_settings[user_Profile].fine == 0)
	{
		// 1. Limit to allowed step range
		// 2. Cycle up and at top, cycle back down, do nto roll over.
		if (_fndx <= 1)
		{
			_fndx = 1;
			direction = 1;   // cycle upwards
		}

		if (_fndx >= TS_STEPS-1)
		{
			_fndx = TS_STEPS-1;
			direction = -1;
		}
		
		if (swiped == 0)
			_fndx += direction; // Index our step up or down
		else	
			_fndx += swiped;  // forces a step higher or lower then current
		
		if (_fndx > TS_STEPS-1)   // ensure we are still in range
			_fndx = TS_STEPS - 1;  // just in case it over ranges, bad stuff happens when it does
		if (_fndx < 1)
			_fndx = 1;  // just in case it over ranges, bad stuff happens when it does		
	}

	if (user_settings[user_Profile].fine && swiped == -1)  // 1 Hz steps
		bandmem[curr_band].tune_step = 0;   // set to 1 hz steps
	else if (user_settings[user_Profile].fine && swiped == 1)
		bandmem[curr_band].tune_step = 1;    // normally swiped is +1 or -1
	else if (user_settings[user_Profile].fine && swiped == 0)
	{
		if (_fndx > 0)
			_fndx = 0;			
		else
			_fndx = 1;
		bandmem[curr_band].tune_step = _fndx;
	}
	else 
		bandmem[curr_band].tune_step = _fndx;  // Fine tunig mode is off, allow all steps 10hz and higher
     
    Serial.print("Set Rate to ");
    Serial.println(tstep[bandmem[curr_band].tune_step].ts_name);
    displayRate();
}

// AGC button
void AGC()
{
    selectAgc(bandmem[curr_band].agc_mode + 1);            
    Serial.print("Set AGC to ");
    Serial.println(bandmem[curr_band].agc_mode);            
    sprintf(std_btn[AGC_BTN].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
    sprintf(labels[AGC_LBL].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
    displayAgc();
}

// MUTE
void Mute()
{  
    if (user_settings[user_Profile].spkr_en)
    {
        if (!user_settings[user_Profile].mute)
        {
            RampVolume(0.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"           
            user_settings[user_Profile].mute = ON;
        }
        else    
        {    //codec1.muteHeadphone();
            RampVolume(1.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"                        
            user_settings[user_Profile].mute = OFF;        
        }
        displayMute();
    }        
}

// MENU
void Menu()
{   
    popup = 1;
    pop_win(1);
    Sp_Parms_Def[spectrum_preset].spect_wf_colortemp += 10;
    if (Sp_Parms_Def[spectrum_preset].spect_wf_colortemp > 10000)
        Sp_Parms_Def[spectrum_preset].spect_wf_colortemp = 1;              
    Serial.print("spectrum_wf_colortemp = ");
    Serial.println(Sp_Parms_Def[spectrum_preset].spect_wf_colortemp); 
    displayMenu();
    Serial.println("Menu Pressed");
}

// VFO A/B
void VFO_AB()
{
    if (bandmem[curr_band].VFO_AB_Active == VFO_A)
    {
        bandmem[curr_band].VFO_AB_Active = VFO_B;
        selectMode(bandmem[curr_band].mode_B);
    }
    else if (bandmem[curr_band].VFO_AB_Active == VFO_B)
    {
        bandmem[curr_band].VFO_AB_Active = VFO_A;
        selectMode(bandmem[curr_band].mode_A);
    }
    VFOA = bandmem[curr_band].vfo_A_last;
    VFOB = bandmem[curr_band].vfo_B_last;
    selectFrequency(0);
    displayVFO_AB();
    displayMode();
    Serial.print("Set VFO_AB_Active to ");
    Serial.println(bandmem[curr_band].VFO_AB_Active,DEC);
}

// ATT
//   toogle = -1 sets attenuator state to current database value. Used for startup or changing bands.
//   toogle = 0 sets attenuator state off
//   toogle = 1 sets attenuator state on
//   toogle = 2 toggles attenuator state
//   dB = 1-31.  Set att relay on and set attenuattion level.  0-31 is a valid range to set but we will only use 1-31
//
void Atten(int8_t toggle)
{
    // Set the attenuation level from the value in the database
    #ifdef DIG_STEP_ATT
      setAtten_dB(bandmem[curr_band].attenuator_dB);  // set attenuator level to value in database for this band
    #endif    
    if (toggle == 2)    // toggle if ordered, else just set to current state such as for startup.
    {
        if (bandmem[curr_band].attenuator)  // toggle the attenuator tracking state
            bandmem[curr_band].attenuator = ATTEN_OFF;
        else 
            bandmem[curr_band].attenuator = ATTEN_ON;
    }
    else if (toggle == 1)    // toggle if ordered, else just set to current state such as for startup.
        bandmem[curr_band].attenuator = ATTEN_ON;  // le the attenuator tracking state to ON
    else if (toggle == 0)    // toggle if ordered, else just set to current state such as for startup.
        bandmem[curr_band].attenuator = ATTEN_OFF;  // set attenuator tracking state to OFF
    // any other value of toggle pass through with unchanged state, jsut set the relays to current state
    
    #ifdef SV1AFN_BPF
      bpf.setAttenuator((bool) bandmem[curr_band].attenuator);  // Turn attenuator relay on or off
    #endif
    displayAttn();
    Serial.print("Set Attenuator Relay to ");
    Serial.print(bandmem[curr_band].attenuator);
    Serial.print(" and dB value to ");
    Serial.println(bandmem[curr_band].attenuator_dB);
}

// PREAMP button
//   toogle = 0 sets Preamp state off
//   toogle = 1 sets Preamp state on
//   toogle = 2 toggles Preamp state
//   toogle = -1 or any value other than 0-2 sets Preamp state to current database value. Used for startup or changing bands.
//
void Preamp(int8_t toggle)
{
    if (toggle == 2)    // toggle state
    {
        if (bandmem[curr_band].preamp == PREAMP_ON)
            bandmem[curr_band].preamp = PREAMP_OFF;
        else 
            bandmem[curr_band].preamp = PREAMP_ON;
    }
    else if (toggle == 1)  // set to ON
        bandmem[curr_band].preamp = PREAMP_ON;
    else if (toggle == 0)  // set to OFF
        bandmem[curr_band].preamp = PREAMP_OFF;
    // any other value of toggle pass through with unchanged state, jsut set the relays to current state
    
    #ifdef SV1AFN_BPF
      bpf.setPreamp((bool) bandmem[curr_band].preamp);
    #endif
    displayPreamp();
    Serial.print("Set Preamp to ");
    Serial.println(bandmem[curr_band].preamp);
}

// RIT button
void RIT()
{
    if (bandmem[curr_band].RIT_en == ON)
        bandmem[curr_band].RIT_en = OFF;
    else if (bandmem[curr_band].RIT_en == OFF)
        bandmem[curr_band].RIT_en = ON;
    displayRIT();
    Serial.print("Set RIT to ");
    Serial.println(bandmem[curr_band].RIT_en);
}
    
// XIT button
void XIT()
{
    if (bandmem[curr_band].XIT_en == ON)
        bandmem[curr_band].XIT_en = OFF;
    else if (bandmem[curr_band].XIT_en == OFF)
        bandmem[curr_band].XIT_en = ON;
    displayXIT();
    Serial.print("Set XIT to ");
    Serial.println(bandmem[curr_band].XIT_en);
}

// SPLIT button
void Split()
{
    if (bandmem[curr_band].split == ON)
        bandmem[curr_band].split = OFF;
    else if (bandmem[curr_band].split == OFF)
        bandmem[curr_band].split = ON;
    displaySplit();
    displayFreq();
    Serial.print("Set Split to ");
    Serial.println(bandmem[curr_band].split);

}

// XVTR button
void Xvtr()
{
    if (bandmem[curr_band].xvtr_en== ON)
        bandmem[curr_band].xvtr_en = OFF;
    else if (bandmem[curr_band].xvtr_en == OFF)
        bandmem[curr_band].xvtr_en = ON;
    //
    //   Insert any future hardware setup calls
    //
    displayXVTR();
    Serial.print("Set Xvtr Enable to ");
    Serial.println(bandmem[curr_band].xvtr_en);
}

// ATU button
void ATU()
{    
    if (bandmem[curr_band].ATU== ON)
        bandmem[curr_band].ATU = OFF;
    else if (bandmem[curr_band].ATU == OFF)
        bandmem[curr_band].ATU = ON;    
    //
    //   Insert any future ATU hardware setup calls
    //
    displayATU();
    Serial.print("Set ATU to ");
    Serial.println(bandmem[curr_band].ATU);
}

// Fine button
void Fine()
{
    extern uint8_t enc_ppr_response;        

    if (user_settings[user_Profile].fine== ON)
    {
        user_settings[user_Profile].fine = OFF;
        enc_ppr_response /= 1.4;
    }
    else if (user_settings[user_Profile].fine == OFF)
    {
        user_settings[user_Profile].fine = ON;
        enc_ppr_response *= 1.4;
    }
    Rate(0);   
    displayFine();
    displayRate();
    
    Serial.print("Set Fine to ");
    Serial.println(user_settings[user_Profile].fine);
}

// ANT button
void Ant()
{
    if (bandmem[curr_band].ant_sw== 1)
        bandmem[curr_band].ant_sw = 2;
    else if (bandmem[curr_band].ant_sw == 2)
        bandmem[curr_band].ant_sw = 1;
    displayANT();
    Serial.print("Set Ant Sw to ");
    Serial.println(bandmem[curr_band].ant_sw);

#ifdef DIG_STEP_ATT
// FOR TEST of Attenuator settings
static int i=1;
i = bandmem[curr_band].attenuator_dB +1;
if (i> 31)
    i=1;
if (i< 1)
    i= 31;
setAtten_dB(i);
bandmem[curr_band].attenuator_dB = i;
Serial.println(i);
#endif

}

// XMIT button
void Xmit()
{
    if (user_settings[user_Profile].xmit== ON)
        user_settings[user_Profile].xmit = OFF;
    else if (user_settings[user_Profile].xmit == OFF)
        user_settings[user_Profile].xmit = ON;
    displayXMIT();
    displayFreq();
    Serial.print("Set XMIT to ");
    Serial.println(user_settings[user_Profile].xmit);
}

// NB button
void NB()
{
    if (user_settings[user_Profile].nb_en > NBOFF)
        user_settings[user_Profile].nb_en = NBOFF;
    else if (user_settings[user_Profile].nb_en == NBOFF)
        user_settings[user_Profile].nb_en = NB1;
    displayNB();
    Serial.print("Set NB to ");
    Serial.println(user_settings[user_Profile].nb_en);
}
    
// NR button
void NR()
{
    if (user_settings[user_Profile].nr_en > NROFF)
        user_settings[user_Profile].nr_en = NROFF;
    else if (user_settings[user_Profile].nr_en == NROFF)
        user_settings[user_Profile].nr_en = NR1;
    displayNR();
    Serial.print("Set NR to ");
    Serial.println(user_settings[user_Profile].nr_en);
}

// Enet button
void Enet()
{
    if (user_settings[user_Profile].enet_output== ON)
        user_settings[user_Profile].enet_output = OFF;
    else if (user_settings[user_Profile].enet_output == OFF)
        user_settings[user_Profile].enet_output = ON;
    displayEnet();
    Serial.print("Set Ethernet to ");
    Serial.println(user_settings[user_Profile].enet_output);
}

// Spot button
void Spot()
{
    if (user_settings[user_Profile].spot== ON)
        user_settings[user_Profile].spot = OFF;
    else if (user_settings[user_Profile].spot == OFF)
        user_settings[user_Profile].spot = ON;
    displaySpot();
    Serial.print("Set Spot to ");
    Serial.println(user_settings[user_Profile].spot);
// adjust ref Floor   
Sp_Parms_Def[spectrum_preset].spect_floor += 5;
if (Sp_Parms_Def[spectrum_preset].spect_floor > -130)
    Sp_Parms_Def[spectrum_preset].spect_floor = -220; 
//Serial.println(Sp_Parms_Def[spectrum_preset].spect_floor);
}

// Notch button
void Notch()
{
    if (user_settings[user_Profile].notch== ON)
        user_settings[user_Profile].notch = OFF;
    else if (user_settings[user_Profile].notch == OFF)
        user_settings[user_Profile].notch = ON;
    displayNotch();
    Serial.print("Set Notch to ");
    Serial.println(user_settings[user_Profile].notch);
}

// BAND UP button
void BandUp()
{
    changeBands(1);
    displayBandUp();
    Serial.print("Set Band UP to ");
    Serial.println(bandmem[curr_band].band_num,DEC);
}

// BAND DOWN button
void BandDn()
{
    Serial.println("BAND DN");
    changeBands(-1);
    displayBandDn();
    Serial.print("Set Band DN to ");
    Serial.println(bandmem[curr_band].band_num,DEC);
}

// BAND button
void Band()
{
    popup = 1;
    pop_win(1);
    changeBands(1);  // increment up 1 band for now until the pop up windows buttons and/or MF are working
    displayBand();
    Serial.print("Set Band to ");
    Serial.println(bandmem[curr_band].band_num,DEC);
}

// DISPLAY button
void Display()
{   
    if (Sp_Parms_Def[spectrum_preset].spect_dot_bar_mode)
    {
        display_state = 0;
        Sp_Parms_Def[spectrum_preset].spect_dot_bar_mode = 0;
    }
    else 
    {
        display_state = 1;
        Sp_Parms_Def[spectrum_preset].spect_dot_bar_mode = 1;
    }
    popup = 1;
    pop_win(1);
    displayDisplay();
    Serial.print("Set Display Button to ");
    Serial.println(display_state);
}

/*******************************************************************************
* Function Name: setAtten_dB()
********************************************************************************
*
* Summary:
* Main function performs following functions:
* 1: Configures the solid state attenuator by shifting 16 bits of address and
*    atten level in LSB first.
* 
* Parameters:
*  atten = attenuation level to set in range of 0 to 31 (in dB)
*
* Return:
*  None.
*
*******************************************************************************/
void setAtten_dB(uint8_t atten)
{
    #ifdef DIG_STEP_ATT
    uint8_t   i;
    char    atten_str[8] = {'\0'};
    char    atten_data[8] = {'\0'};

    if(atten > 31) 
        atten = 31;
    if(atten <= 0)
        atten = 0;
    atten *= 2; //shift the value x2 so the LSB controls the 0.5 step.  We are not using the 0.5 today.
    /* Convert to 8 bits of  0 and 1 format */
    itoa(atten, atten_str, 2);
    
    // pad with leading 0s as needed.  6 bits for the PE4302
    for(i=0;(i<6-strlen(atten_str));i++)
    {
        atten_data[i]='0';
    }
    strncat(atten_data, atten_str, strlen(atten_str));

    //  LE = 0 to allow writing data into shift register
    digitalWrite(Atten_LE,   (uint8_t) OFF);
    digitalWrite(Atten_DATA, (uint8_t) OFF);
    digitalWrite(Atten_CLK,  (uint8_t) OFF);
     delayMicroseconds(10);
    //  Now loop for 6 bits, set data on Data pin and toggle Clock pin.  
    //    Start with the MSB first so start at the left end of the string   
    for(i=0;i<6;i++)
    {
        // convert ascii 0 or 1 to a decimal 0 or 1 
        digitalWrite(Atten_DATA, (uint8_t) atten_data[i]-48);
        delayMicroseconds(10);
        digitalWrite(Atten_CLK,  (uint8_t) ON);
        delayMicroseconds(10);
        digitalWrite(Atten_CLK,  (uint8_t) OFF); 
        delayMicroseconds(10);
    }
    //  Toggle LE pin to latch the data and set the new attenuation value in the hardware
    digitalWrite(Atten_LE, (uint8_t) ON);
    delayMicroseconds(10);
    digitalWrite(Atten_LE, (uint8_t) OFF);
    return;    
    #endif
}