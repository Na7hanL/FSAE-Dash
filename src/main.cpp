#include <Arduino.h>
#include <SPI.h>
#include <stdarg.h>
// Definitions for our circuit board and display.
#include "CFA10100_defines.h"

// test commit

#if BUILD_SD
#include <SD.h>
#endif

// The very simple EVE library files
#include "EVE_base.h"
#include "EVE_draw.h"

// Our demonstrations of various EVE functions
//#include "demos.h"
#include "dash.h"


  
//===========================================================================
void setup()
  {
#if (DEBUG_LEVEL != DEBUG_NONE)
  // Initialize UART for debugging messages
  Serial.begin(115200);
#endif // (DEBUG_LEVEL != DEBUG_NONE)
  DBG_STAT("Begin\n");

  //Initialize GPIO port states
  // Set CS# high to start - SPI inactive
  SET_EVE_CS_NOT;
  // Set PD# high to start
  SET_EVE_PD_NOT;
  SET_SD_CS_NOT;

  //Initialize port directions
  // EVE interrupt output (not used in this example)
  pinMode(EVE_INT, INPUT_PULLUP);
  // EVE Power Down (reset) input
  pinMode(EVE_PD_NOT, OUTPUT);
  // EVE SPI bus CS# input
  pinMode(EVE_CS_NOT, OUTPUT);
  // USD card CS
  pinMode(SD_CS, OUTPUT);
  // Optional pin used for LED or oscilloscope debugging.
  pinMode(DEBUG_LED, OUTPUT);

  // Initialize SPI
  SPI.begin();
  //Bump the clock to 8MHz. Appears to be the maximum.
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  DBG_GEEK("SPI initialzed to: 8MHz\n");
\

  //See if we can find the FTDI/BridgeTek EVE processor
  if(0 != EVE_Initialize())
    {
    DBG_STAT("Failed to initialize %s8%02X. Stopping.\n",EVE_DEVICE<0x14?"FT":"BT",EVE_DEVICE);
    while(1);
    }
  else
    {
    DBG_STAT("%s8%02X initialized.\n",EVE_DEVICE<0x14?"FT":"BT",EVE_DEVICE);
    }
  } //  setup()
//===========================================================================
void loop()
  {
  DBG_GEEK("Loop initialization.\n");

  //Get the current write pointer from the EVE
  uint16_t
    FWo;
  FWo = EVE_REG_Read_16(EVE_REG_CMD_WRITE);
  DBG_GEEK("Initial Offset Read: 0x%04X = %u\n",FWo ,FWo);

  //Keep track of the RAM_G memory allocation
  uint32_t
    RAM_G_Unused_Start;
  RAM_G_Unused_Start=0;
  DBG_GEEK("Initial RAM_G: 0x%08lX = %lu\n",RAM_G_Unused_Start,RAM_G_Unused_Start);

  // We need to keep track of the bitmap handles and where they are used.
  //
  // By default, bitmap handles 16 to 31 are used for built-in font and 15
  // is used as scratch bitmap handle by co-processor engine commands
  // CMD_GRADIENT, CMD_BUTTON and CMD_KEYS.
  //
  // For whatever reason, I am going to allocate handles from 14 to 0.
  uint8_t
    next_bitmap_handle_available;
  next_bitmap_handle_available=14;

  DBG_GEEK("EVE_Initialize_Flash() . . . ");
  FWo=EVE_Initialize_Flash(FWo);
  DBG_GEEK("done.\n");

  uint8_t
    flash_status;
  flash_status = EVE_REG_Read_8(EVE_REG_FLASH_STATUS);
  DBG_GEEK_Decode_Flash_Status(flash_status);

#if (0 != PROGRAM_FLASH_FROM_USD)
  //Keep track of the current write pointer into flash.
  uint32_t
    Flash_Sector;
  Flash_Sector=0;

  //Load the BLOB & write our image data to the flash from
  //the uSD card. This only needs to be executed once. It
  //uses RAM_G as scratch temporary memory, but does not
  //allocate any RAM_G.
  FWo= Initialize_Flash_From_uSD(FWo,
                                 RAM_G_Unused_Start,
                                 &Flash_Sector);
#else  // (0 != PROGRAM_FLASH_FROM_USD)
  DBG_GEEK("Not programming flash.\n");
#endif // (0 != PROGRAM_FLASH_FROM_USD)


#if (0 != FSAE_DASH)
  bool dashMode = true;
  int flip = 0;
  DBG_STAT("Initialize_Dash() . . .");
  Initialize_Dash();
  DBG_STAT(" done.\n");
  
   //Bitmask of valid points in the array
  uint8_t
    points_touched_mask;
#if (EVE_TOUCH_TYPE == EVE_TOUCH_RESISTIVE)
  DBG_GEEK("Resistive touch, single point.\n");
  int16_t
    x_points[1];
  int16_t
    y_points[1];
#endif // (EVE_TOUCH_TYPE == EVE_TOUCH_RESISTIVE)

#if (EVE_TOUCH_TYPE == EVE_TOUCH_CAPACITIVE)
  DBG_GEEK("Capacitive touch, multiple points.\n");
  int16_t
    x_points[5];
  int16_t
    y_points[5];
#endif // (EVE_TOUCH_TYPE == EVE_TOUCH_CAPACITIVE)

#endif // FSAE_DASH

  DBG_STAT("Initialization complete, entering main loop.\n");

  while(1)
    {
    //========== FRAME SYNCHRONIZING ==========
    // Wait for graphics processor to complete executing the current command
    // list. This happens when EVE_REG_CMD_READ matches EVE_REG_CMD_WRITE, indicating
    // that all commands have been executed.  We have a local copy of
    // EVE_REG_CMD_WRITE in FWo.
    //
    // This appears to only occur on frame completion, which is nice since it
    // allows us to step the animation along at a reasonable rate.
    //
    // If possible, I have tweaked the timing on the Crystalfontz displays
    // to all have ~60Hz frame rate.
    FWo=Wait_for_EVE_Execution_Complete(FWo);


    //========== START THE DISPLAY LIST ==========
    // Start the display list
    FWo=EVE_Cmd_Dat_0(FWo,
                      (EVE_ENC_CMD_DLSTART));
  
    // Set the default clear color to black
    FWo=EVE_Cmd_Dat_0(FWo,
                      EVE_ENC_CLEAR_COLOR_RGB(0,0,0));
    // Clear the screen - this and the previous prevent artifacts between lists
    FWo=EVE_Cmd_Dat_0(FWo,
                      EVE_ENC_CLEAR(1 /*CLR_COL*/,1 /*CLR_STN*/,1 /*CLR_TAG*/));
    //========== ADD GRAPHIC ITEMS TO THE DISPLAY LIST ==========
    //Fill background with white
    FWo = EVE_Cmd_Dat_0(FWo, EVE_ENC_COLOR_RGB(0,0,0));
    FWo=EVE_Filled_Rectangle(FWo,0,0,LCD_WIDTH-1,LCD_HEIGHT-1);
                            

#if (0 != FSAE_DASH)
  FWo = Add_Dash_To_Display_List(FWo, dashMode);
  
  points_touched_mask = Read_Touch(x_points,y_points);

  if(0 != points_touched_mask){
      flip += 1;
  }
  else{
    flip = 0;
  }

  if(flip >= 100){
    flip = 0;
    dashMode = !dashMode;
    delay(100);
  }
  
  DBG_GEEK("%d", flip);
  DBG_GEEK("\n");

#endif //DASH_DEMO

#if (0 != REMOTE_BACKLIGHT_DEBUG)
    int
      byte_read;
    if(-1 != (byte_read=Serial.read()))
      {
      DBG_GEEK("Serial Data Read: %3d 0x%02X",byte_read,byte_read);
      if(byte_read<=128)
        {
        EVE_REG_Write_8(EVE_REG_PWM_DUTY,byte_read);
        DBG_GEEK(", backlight set.");
        }
      DBG_GEEK("\n");
      }
#endif // (0 != REMOTE_BACKLIGHT_DEBUG)

    //========== FINSH AND SHOW THE DISPLAY LIST ==========
    // Instruct the graphics processor to show the list
    FWo=EVE_Cmd_Dat_0(FWo, EVE_ENC_DISPLAY());
    // Make this list active
    FWo=EVE_Cmd_Dat_0(FWo, EVE_ENC_CMD_SWAP);
    // Update the ring buffer pointer so the graphics processor starts executing
    EVE_REG_Write_16(EVE_REG_CMD_WRITE, (FWo));

#if (0 != FSAE_DASH)
  updateData();
  
#endif //DASH_DEMO

    }  // while(1)
  } // loop()
//===========================================================================