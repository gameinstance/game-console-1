#include <Arduino.h>
#include <SPI.h>
#include <LCDNokia5100.h>


/*
 * A simple alias for the display class.
 */
typedef LCDNokia5100 Display;


/*
 * The game console storage class.
 */
class Storage {
    
    public:
        
        /// default constructor
        Storage();
        /// destructor
        virtual ~Storage();
        
        /// initializes the object
        void Init(unsigned char gameIndex);
        /// gets the high score
        unsigned int GetScore();
        /// sets the high score
        void SetScore(unsigned int value);
        
        
    private:
        
        /// the per-game storage chunk size
        static const unsigned char STORAGE_CHUNK_SIZE = 4;
        
        /// the game index
        unsigned int m_gameIndex;
};


/*
 * The base game console class.
 */
class GameConsole {

  public:

    /// the display width in pixels
    static const byte WIDTH = Display::WIDTH;
    /// the display height in pixels
    static const byte HEIGHT = Display::HEIGHT;
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
    /// the game index
    virtual unsigned char GameIndex() = 0;

    /// the center points
    int m_centerX, m_centerY;
    /// the display
    Display m_lcd;
  
    
  protected:
      
    /// the storage object
    Storage m_storage;
    

  private:

    /// indicates the button index
    byte ButtonIndex(byte button);

    /// the array of button states
    bool m_bPress[BUTTON_COUNT];
};
