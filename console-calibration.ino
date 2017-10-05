#include <GameConsole.h>

class Calibration : public GameConsole {

  public:
    /// default constructor
    Calibration() : GameConsole(), 
      m_state(0), 
      m_nowTime(0), 
      m_lastTime(0) {
      //
    };
    /// destructor
    virtual ~Calibration() {
      //
    };
    /// the main method implementation
    void Execute() {
      //
      m_nowTime = millis();
      if (m_state == 0) {
        // initial game state
        m_lastTime = m_nowTime;
        m_state = 1;
      }
      
      if (m_state == 1) {
        // reading input
        ClearDisplay();
        if (IsPressed(BUTTON_A)) {
          // 
          m_lcd.Text("A", 8, 32, true);
        }
        if (IsPressed(BUTTON_B)) {
          // 
          m_lcd.Text("B", 68, 32, true);
        }
        if (IsPressed(BUTTON_C)) {
          // 
          m_lcd.Text("C", 68, 8, true);
        }
        m_lcd.Text("X:", 24, 16, true);
        String s = String(GetAxis(AXIS_X));
        m_lcd.Text(s, 36, 16, true);
        m_lcd.Text("Y:", 24, 24, true);
        s = String(GetAxis(AXIS_Y));
        m_lcd.Text(s, 36, 24, true);
        m_lcd.Update();
        m_state = 2;
      }
      
      if (m_state == 2) {
        // waiting state
        if (m_nowTime > m_lastTime + 40) {
          // 
          m_lastTime = m_nowTime; 
          m_state = 1;
        }        
      }
    }


  private:

    /// the state of the automate
    byte m_state;
    /// high-res time probes
    unsigned long m_nowTime, m_lastTime;
};

/// the game instance
Calibration game;

void setup() {
  // put your setup code here, to run once:
  game.Setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  game.Loop();
}
