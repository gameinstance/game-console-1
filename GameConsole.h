#include <Arduino.h>
#include <SPI.h>
#include <LCDNokia5100.h>

/*
 * The base game console class.
 */
class GameConsole {

  public:

    /// the display width in pixels
    static const byte WIDTH = LCDNokia5100::WIDTH;
    /// the display height in pixels
    static const byte HEIGHT = LCDNokia5100::HEIGHT;
    /// the number of buttons in use
    static const byte BUTTON_COUNT = 3;
    /// button A
    static const byte BUTTON_A = 10;
    /// button B
    static const byte BUTTON_B = 12;
    /// button C
    static const byte BUTTON_C = A3;
    /// the X axis
    static const byte AXIS_X = A2;
    /// the Y axis
    static const byte AXIS_Y = A1;

    /// default constructor
    GameConsole();
    /// destructor
    virtual ~GameConsole();

    /// sets up the game console object
    void Setup();
    /// clears the display
    void ClearDisplay();
    /// indicates that a button was pressed once
    bool WasPressed(byte button);
    /// indicates once that button A is pressed
    bool IsPressed(byte button);
    /// indicates the joystick position on axis
    int GetAxis(byte axis);
    /// the loop method
    void Loop();
    /// the game state machine 
    virtual void Execute() = 0;

    /// the center points
    int m_centerX, m_centerY;
    /// the display
    LCDNokia5100 m_lcd;


  private:

    /// indicates the button index
    byte ButtonIndex(byte button);

    /// the array of button states
    bool m_bPress[BUTTON_COUNT];
};
