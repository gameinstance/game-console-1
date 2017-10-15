#include <GameConsole.h>

struct Point {
  
  unsigned char m_x, m_y;

  void Init(unsigned char x, unsigned char y) {
    //
    m_x = x;
    m_y = y;
  }
};

struct Sane {

  signed char m_integrity;
  
  void Init(unsigned char integrity) {
    // 
    m_integrity = integrity;
  }
  bool IsAlive() {
    // 
    return (m_integrity > 0);
  }
  bool JustDied() {
    // 
    return ((m_integrity <= 0) && (m_integrity > -8));
  }
  void Hit(byte degree) {
    // 
    m_integrity -= degree;
  }
};

struct SanePoint: public Sane, public Point {

  void Init(unsigned char x, unsigned char y) {
    //
    Point::Init(x, y);
    Sane::Init(100);
  }
};

struct Invader : public SanePoint {

  static const byte THICKNESS = 8;
  static const char MASK[4];
};

const char Invader::MASK[4] = {0x11, 0x7A, 0xB4, 0x3C};

struct Ship : public SanePoint {

  static const byte THICKNESS = 8;
  static const char MASK[4];
};

const char Ship::MASK[4] = {0x80, 0xC0, 0xC0, 0xF0};

struct Explosion {

  static const byte THICKNESS = 7;
  static const char MASK[4];
};

const char Explosion::MASK[4] = {0x49, 0x2A, 0x14, 0x63};

struct Shield : public SanePoint {

  static const byte THICKNESS = 10;

  bool IsStanding() {
    // 
    return IsAlive();
  }
};

struct LaserBeam : public Point {

  /// default constructor
  LaserBeam() {
    // initialize the y coordinate outside visible frame
    m_y = 255;
  }  
  /// indicates that beam is visible
  bool IsVisible() {
    //
    return (m_y < Display::HEIGHT - 11);
  }
};

class SpaceInvaders : public GameConsole {

  public:

    /// always-the-same random sequence
    static const unsigned char RANDOM[16];
  
    /// default constructor
    SpaceInvaders() : GameConsole(), 
      m_state(0), 
      m_nowTime(0), 
      m_lastTime(0), 
      m_invaderSpeed(0),
      m_joysickX(0), 
      m_invaderLaserIndex(0), 
      m_randomIndex(0), 
      m_score(0), m_level(0), 
      m_lives(0), m_stepDown(0),
      m_invaderCount(0), 
      m_invaderAdvancePeriod(8) {
      //
    };
    /// destructor
    virtual ~SpaceInvaders() {
      // 
    };

    /// the game index
    unsigned char GameIndex() {
      // 
      return 1;
    }
    /// game setup method
    void Setup() {
      // 
      GameConsole::Setup();
      m_lcd.Contrast(45);
      m_lcd.Fill(false);
      m_lcd.Text("SPACE", 28, 10, true);
      m_lcd.Text("INVADERS", 19, 22, true);
      m_lcd.Update();
      delay(2000);
      if (IsPressed(BUTTON_A) && IsPressed(BUTTON_B)) {
        // reseting high score
        m_storage.SetScore(0);
      }
    }
    /// the main method implementation
    void Execute() {
      //
      m_nowTime = millis();
      
      if (m_state == 0) {
        // initial game state
        m_lastTime = m_nowTime;
        m_timeDivider = 0;
        m_level = 0;
        m_lives = 3;
        m_score = 0;
        m_state = 10;
      }
      
      if (m_state == 1) {
        // reading input
        if (WasPressed(BUTTON_B)) {
          // pause 
          m_state = 11;
          return;
        }
        m_joysickX = GetAxis(AXIS_X) / 160;
        m_state = 2;
      }

      if (m_state == 2) {
        // fire
        if (WasPressed(BUTTON_A) && !m_beam.IsVisible()) {
          // fire one
          m_beam.Init(m_ship.m_x + Ship::THICKNESS / 2 - 1, m_ship.m_y + Ship::THICKNESS / 2);
        }
        
        if (m_timeDivider % 128 == 0) {
          // 
          if (m_invaderCount > 0) {
            //
            unsigned char invaderIndex = RANDOM[m_randomIndex] % m_invaderCount;
            signed char j = -1;
            for (unsigned char i = 0; i < 16; i ++) {
              // 
              if (m_invaders[i].IsAlive()) {
                // 
                j ++;
              }
              if (j == invaderIndex) {
                // 
                m_invaderBeams[m_invaderLaserIndex].Init(
                  m_invaders[i].m_x + Invader::THICKNESS / 2 - 1, 
                  m_invaders[i].m_y + 6);
                m_invaderLaserIndex ++;
                if (m_invaderLaserIndex >= 2) {
                  // 
                  m_invaderLaserIndex = 0;
                }
                m_randomIndex ++;
                if (m_randomIndex >= 16) {
                  // 
                  m_randomIndex = 0;
                }
                break;
              }                
            }
          }
        }
        m_state = 3;
      }

      if (m_state == 3) {
        // moving
        
        if (m_ship.m_x + m_joysickX < 1) {
          // 
          m_ship.m_x = 1;
        } else if (m_ship.m_x + m_joysickX + Ship::THICKNESS >= Display::WIDTH - 1) {
          // 
          m_ship.m_x = Display::WIDTH - Ship::THICKNESS - 1;
        } else {
          // 
          m_ship.m_x += m_joysickX;
        }

        // moving beams
        if (m_timeDivider % 1 == 0) {
          // 
          if (m_beam.IsVisible()) {
            // 
            m_beam.m_y --;
          }
        }

        // moving invader beams
        if (m_timeDivider % 2 == 0) {
          // 
          for (byte i = 0; i < 2; i ++) {
            // iterating beams
            if (m_invaderBeams[i].IsVisible()) {
              // 
              m_invaderBeams[i].m_y ++;
            }
          }
        }

        // moving invaders
        if (m_timeDivider % m_invaderAdvancePeriod == 0) {
          // 
          signed char down = 0;
          for (byte k = 0; k < 16; k ++) {
            // iterating the invaders
            if (m_invaders[k].IsAlive()) {
              // alive one
              if ((m_invaderSpeed < 0) && (m_invaders[k].m_x <= 1) 
                || (m_invaderSpeed > 0) && (m_invaders[k].m_x + Invader::THICKNESS >= Display::WIDTH - 1)) {
                // 
                m_invaderSpeed = -m_invaderSpeed;
                down = m_stepDown;
                break;
              }
            }
          }
          for (byte k = 0; k < 16; k ++) {
            // iterating the invaders
            if (m_invaders[k].IsAlive()) {
              // alive one
              if (down > 0) {
                // 
                m_invaders[k].m_y += down;
                if (m_invaders[k].m_y + Invader::THICKNESS >= Display::HEIGHT - 6) {
                  // invaders succeeded
                  m_state = 8;
                  return;
                }
              } else {
                // 
                m_invaders[k].m_x += m_invaderSpeed;
              }
            }
          }
        }
        
        m_state = 4;
      }

      if (m_state == 4) {
        // colisions
        // ship's beam
        if (m_beam.IsVisible()) {
          // visible ones
          for (byte j = 0; j < 16; j ++) {
            // iterate the invaders
            if (m_invaders[j].IsAlive()) {
              // alive ones
              m_colisionDegree = ColisionInvaderBeam(j);
              if (m_colisionDegree > 0) {
                // some sort of colision damage
                m_invaders[j].Hit(m_colisionDegree);
                if (!m_invaders[j].IsAlive()) {
                  // got destroyed
                  m_invaderCount --;
                }
                m_score ++;
                // hide the beam
                m_beam.m_y = 255;
              }
            }
          }
          for (byte j = 0; j < 3; j ++) {
            // iterate the shields
            if (m_shields[j].IsStanding()) {
              // the ones still holding
              m_colisionDegree = ColisionShieldBeam(j);
              if (m_colisionDegree > 0) {
                // some sort of colision damage
                m_shields[j].Hit(m_colisionDegree);
                if (m_score > 0) {
                  // 
                  m_score --;
                }
                // hide the beam
                m_beam.m_y = 255;
              }                
            }
          }
        }
        
        for (byte i = 0; i < 2; i ++) {
          // iterate invader beams
          if (m_invaderBeams[i].IsVisible()) {
            // visible ones
            if (m_ship.IsAlive()) {
              // the ship is alive
              m_colisionDegree = ColisionShipInvaderBeam(i);
              if (m_colisionDegree > 0) {
                // some sort of colision damage
                m_ship.Hit(m_colisionDegree);
                // hide the beam
                m_invaderBeams[i].m_y = 255;
              }
            }
            for (byte j = 0; j < 3; j ++) {
              // 
              if (m_shields[j].IsStanding()) {
                // 
                m_colisionDegree = ColisionShieldInvaderBeam(j, i);
                if (m_colisionDegree > 0) {
                  // some sort of colision damage
                  m_shields[j].Hit(m_colisionDegree);
                  // hide the beam
                  m_invaderBeams[i].m_y = 255;
                }                
              }
            }
          }
        }

        for (byte i = 0; i < 16; i ++) {
          // iterate invaders
          if (m_invaders[i].IsAlive()) {
            // alive ones
            m_colisionDegree = ColisionInvaderShip(i);
            if (m_colisionDegree > 0) {
              // some sort of colision damage
              m_state = 8;
              return;
            }
          }
        }
        m_state = 5;
      }

      if (m_state == 5) {
        // display
        byte state = 6;
        // clear display
        m_lcd.Fill(false);
        m_lcd.Line(0, 0, WIDTH - 1, 0, true);
        m_lcd.Line(0, 0, 0, HEIGHT - 7, true);
        m_lcd.Line(WIDTH - 1, HEIGHT - 7, WIDTH - 1, 0, true);
        m_lcd.Line(WIDTH - 1, HEIGHT - 7, 0, HEIGHT - 7, true);
        
        // drawing invaders
        unsigned char column;
        for (byte k = 0; k < 16; k ++) {
          // iterating the invaders
          if (m_invaders[k].IsAlive()) {
            // alive one
            for (int i = 0; i < Invader::THICKNESS; i ++) {
              // drawing by columns
              column = Invader::MASK[(i < 4) ? i : Invader::THICKNESS - 1 - i];
              for (int j = 0; j < Invader::THICKNESS; j ++) {
                // drawing by rows
                if (column & (0x01 << j)) {
                  // 
                  m_lcd.Point(
                    m_invaders[k].m_x + i, 
                    m_invaders[k].m_y + j, 
                    true);
                }
              }
            }
          } else if (m_invaders[k].JustDied()) {
            // recently deceased
            for (int i = 0; i < Explosion::THICKNESS; i ++) {
              // drawing by columns
              column = Explosion::MASK[(i < 4) ? i : Explosion::THICKNESS - i - 1];
              for (int j = 0; j < Explosion::THICKNESS; j ++) {
                // drawing by rows
                if (column & (0x01 << j)) {
                  // 
                  m_lcd.Point(
                    m_invaders[k].m_x + i, 
                    m_invaders[k].m_y + j, 
                    true);
                } else {
                  // 
                  m_lcd.Point(
                    m_invaders[k].m_x + i, 
                    m_invaders[k].m_y + j, 
                    false);
                }
              }
            }
            m_invaders[k].m_integrity --;
          } else if (m_invaderCount <= 0) {
            // 
            m_state = 10;
            return;
          }
        }

        // drawing the shields
        for (byte k = 0; k < 3; k ++) {
          // 
          if (m_shields[k].IsStanding()) {
            // 
            for (int i = 0; i < Shield::THICKNESS; i += (m_shields[k].m_integrity <= 50 ? 2 : 1)) {
              // 
              m_lcd.Point(m_shields[k].m_x + i, m_shields[k].m_y, true);
            }
          }
        }
        
        // drawing the ship
        if (m_ship.IsAlive()) {
          // alive ship
          for (int i = 0; i < Ship::THICKNESS; i ++) {
            // by columns
            column = Ship::MASK[(i < 4) ? i : Ship::THICKNESS - i - 1];
            for (int j = 4; j < 8; j ++) {
              // by rows
              if (column & (0x01 << j)) {
                // 
                m_lcd.Point(m_ship.m_x + i, m_ship.m_y + j, true);
              }
            }
          }          
        } else if (m_ship.JustDied()) {
          // recently deceased ship
          for (int i = 0; i < Explosion::THICKNESS; i ++) {
            // drawing by columns
            column = Explosion::MASK[(i < 4) ? i : Explosion::THICKNESS - i - 1];
            for (int j = 0; j < Explosion::THICKNESS; j ++) {
              // drawing by rows
              if (column & (0x01 << j)) {
                // 
                m_lcd.Point(
                  m_ship.m_x + i, 
                  m_ship.m_y + j + 2, 
                  true);
              } else {
                // 
                m_lcd.Point(
                  m_ship.m_x + i, 
                  m_ship.m_y + j + 2, 
                  false);
              }
            }
          }
          m_ship.m_integrity --;
          m_lcd.DimLight(m_ship.m_integrity % 2 == 0 ? 32 : 255);
        } else {
          // 
          m_lcd.Light(true);
          state = 7;
        }
        
        // drawing laser beam
        if (m_beam.IsVisible()) {
          // 
          m_lcd.Point(m_beam.m_x + 1, m_beam.m_y, true);
          m_lcd.Point(m_beam.m_x + 1, m_beam.m_y + 1, true);
          m_lcd.Point(m_beam.m_x, m_beam.m_y + 1, true);
          m_lcd.Point(m_beam.m_x, m_beam.m_y + 2, true);
        }

        // drawing invader laser beams
        for (byte i = 0; i < 2; i ++) {
          // 
          if (m_invaderBeams[i].IsVisible()) {
            // 
            m_lcd.Point(m_invaderBeams[i].m_x - 1, m_invaderBeams[i].m_y, true);
            m_lcd.Point(m_invaderBeams[i].m_x, m_invaderBeams[i].m_y + 1, true);
            m_lcd.Point(m_invaderBeams[i].m_x - 1, m_invaderBeams[i].m_y + 2, true);
            m_lcd.Point(m_invaderBeams[i].m_x, m_invaderBeams[i].m_y + 3, true);
          }
        }

        // drawing a ship
        for (int i = 0; i < Ship::THICKNESS; i ++) {
          // by columns
          column = Ship::MASK[(i < 4) ? i : Ship::THICKNESS - i - 1];
          for (int j = 4; j < Ship::THICKNESS; j ++) {
            // by rows
            if (column & (0x01 << j)) {
              // 
              m_lcd.Point(1 + i, 40 + j, true);
            } else {
              // 
              m_lcd.Point(1 + i, 40 + j, false);
            }
          }
        }

        // drawing an invader
        for (int i = 0; i < Invader::THICKNESS; i ++) {
          // by columns
          column = Invader::MASK[(i < 4) ? i : Invader::THICKNESS - i - 1];
          for (int j = 2; j < Invader::THICKNESS - 1; j ++) {
            // by rows
            if (column & (0x01 << j)) {
              // 
              m_lcd.Point(76 + i, 41 + j, true);
            } else {
              // 
              m_lcd.Point(76 + i, 41 + j, false);
            }
          }
        }
        m_tempStr = m_lives;
        Text(m_tempStr, 10, 43, true);
        m_tempStr = m_score * 10;
        Text("00000", 56, 43, true);
        Text(m_tempStr, 76 - m_tempStr.length() * 4, 43, true);
        
        // update display
        m_lcd.Update();
        m_state = state;
      }
      
      if (m_state == 6) {
        // waiting state
        if (m_nowTime > m_lastTime + 40) {
          // 
          m_lastTime = m_nowTime;
          m_timeDivider ++;
          m_state = 1;
        }        
      }

      if (m_state == 7) {
        // dead ship
        m_ship.Init(6, 32);
        m_lives --;
        if (m_lives <= -1) {
          // game over
          m_state = 8;
        } else {
          // keep fighting
          m_state = 1;
        }
      }

      if (m_state == 8) {
        // game over
        m_lcd.Text("Game", 30, 14, true);
        m_lcd.Text("Over", 30, 23, true);
        m_lcd.Update();
        delay(3000);
        m_state = 13;
      }

      if (m_state == 9) {
        // end state
        if (IsPressed(BUTTON_A)) {
          // restart
          m_state = 0;
        }
      }

      if (m_state == 10) {
        // next level
        m_level ++;
        m_ship.Init(6, 32);
        for (byte i = 0; i < 3; i ++) {
          //
          m_shields[i].Init(i * Display::WIDTH / 3 + Display::WIDTH / 6 - Shield::THICKNESS / 2, 32);
        }
        m_invaderAdvancePeriod = 8;
        switch (m_level) {
          case 1: {
            //
            m_invaderCount = 16;
            for (byte i = 0; i < m_invaderCount; i ++) {
              // 
              m_invaders[i].Init(
                6 + (Invader::THICKNESS + 1) * (i % 8), 
                2 + (Invader::THICKNESS + 1) * (i / 8)
              );
            }
            m_invaderSpeed = 1;
            m_stepDown = 0;
            break;
          }
          case 2: {
            //
            m_invaderCount = 16;
            for (byte i = 0; i < m_invaderCount; i ++) {
              // 
              m_invaders[i].Init(
                6 + (Invader::THICKNESS + 1) * (i % 8), 
                2 + (Invader::THICKNESS + 1) * (i / 8)
              );
            }
            m_invaderSpeed = 1;
            m_stepDown = 1;
            break;
          }
          case 3: {
            //
            m_invaderCount = 16;
            for (byte i = 0; i < m_invaderCount; i ++) {
              // 
              m_invaders[i].Init(
                6 + (Invader::THICKNESS + 1) * (i % 8), 
                2 + (Invader::THICKNESS + 1) * (i / 8)
              );
            }
            m_invaderAdvancePeriod = 6;
            m_invaderSpeed = 1;
            m_stepDown = 1;
            break;
          }
          case 4: {
            //
            m_invaderCount = 16;
            for (byte i = 0; i < m_invaderCount; i ++) {
              // 
              m_invaders[i].Init(
                6 + (Invader::THICKNESS + 1) * (i % 8), 
                2 + (Invader::THICKNESS + 1) * (i / 8)
              );
            }
            m_invaderAdvancePeriod = 4;
            m_invaderSpeed = 1;
            m_stepDown = 1;
            break;
          }
          case 5: {
            //
            m_invaderCount = 16;
            for (byte i = 0; i < m_invaderCount; i ++) {
              // 
              m_invaders[i].Init(
                6 + (Invader::THICKNESS + 1) * (i % 8), 
                2 + (Invader::THICKNESS + 1) * (i / 8)
              );
            }
            m_invaderAdvancePeriod = 3;
            m_invaderSpeed = 1;
            m_stepDown = 1;
            break;
          }
          case 6: {
            //
            m_invaderCount = 16;
            for (byte i = 0; i < m_invaderCount; i ++) {
              // 
              m_invaders[i].Init(
                6 + (Invader::THICKNESS + 1) * (i % 8), 
                2 + (Invader::THICKNESS + 1) * (i / 8)
              );
            }
            m_invaderAdvancePeriod = 2;
            m_invaderSpeed = 1;
            m_stepDown = 1;
            break;
          }
          case 7: {
            //
            m_invaderCount = 16;
            for (byte i = 0; i < m_invaderCount; i ++) {
              // 
              m_invaders[i].Init(
                6 + (Invader::THICKNESS + 1) * (i % 8), 
                2 + (Invader::THICKNESS + 1) * (i / 8)
              );
            }
            m_invaderAdvancePeriod = 1;
            m_invaderSpeed = 1;
            m_stepDown = 1;
            break;
          }
          default: {
            //
            m_state = 12;
            return;
          }
        }
        m_lcd.Fill(false);
        m_lcd.Line(0, 0, WIDTH - 1, 0, true);
        m_lcd.Line(0, 0, 0, HEIGHT - 7, true);
        m_lcd.Line(WIDTH - 1, HEIGHT - 7, WIDTH - 1, 0, true);
        m_lcd.Line(WIDTH - 1, HEIGHT - 7, 0, HEIGHT - 7, true);
        m_lcd.Text("Level ", 22, 18, true);
        m_tempStr = m_level;
        m_lcd.Text(m_tempStr, 54, 18, true);
        m_lcd.Update();
        delay(2000);
        m_state = 1;
      }

      if (m_state == 11) {
        // pause state
        if (WasPressed(BUTTON_B)) {
          // 
          m_state = 1;
        }
      }

      if (m_state == 12) {
        // game complete
        m_lcd.Fill(false);
        m_lcd.Line(0, 0, WIDTH - 1, 0, true);
        m_lcd.Line(0, 0, 0, HEIGHT - 7, true);
        m_lcd.Line(WIDTH - 1, HEIGHT - 7, WIDTH - 1, 0, true);
        m_lcd.Line(WIDTH - 1, HEIGHT - 7, 0, HEIGHT - 7, true);
        m_lcd.Text("Game complete", 3, 10, true);
        m_lcd.Text("Congrats!", 15, 22, true);
        m_lcd.Update();
        delay(3000);
        m_state = 13;
      }

      if (m_state == 13) {
        // scores
        m_lcd.Fill(false);
        m_lcd.Text("Score", 2, 24, true);
        m_tempStr = m_score * 10;
        m_lcd.Text(m_tempStr, 44, 24, true);
        int highScore = m_storage.GetScore();
        m_tempStr = highScore * 10;
        if (m_score > highScore) {
          // 
          m_storage.SetScore(m_score);
          m_lcd.Text("New record!", 2, 2, true);
        }
        m_lcd.Text("High ", 2, 14, true);
        m_lcd.Text(m_tempStr, 44, 14, true);
        m_lcd.Text("press A ", 20, 40, true);
        
        m_lcd.Update();
        
        m_state = 9;
      }
    }


  private:

    /// 0-9 numeric chars as 5x3 pixel fonts
    unsigned char NUMBERS[10][3] = {
      {0x0E, 0x11, 0x0E},   // 0
      {0x12, 0x1F, 0x10},   // 1
      {0x12, 0x19, 0x16},   // 2
      {0x11, 0x15, 0x0A},   // 3
      {0x07, 0x04, 0x1C},   // 4
      {0x17, 0x15, 0x09},   // 5
      {0x0E, 0x15, 0x09},   // 6
      {0x19, 0x05, 0x03},   // 7
      {0x0A, 0x15, 0x0A},   // 8
      {0x12, 0x15, 0x0E},   // 9
    };

    /// invader-ship colision detection    
    byte ColisionInvaderShip(byte j) {
      // 
      return (Colision(
        m_invaders[j].m_x, m_invaders[j].m_y, Invader::THICKNESS, Invader::THICKNESS, 
        m_ship.m_x, m_ship.m_y + 4, Ship::THICKNESS, 4) ? 
        100 : 
        0);
    };
    /// invader-beam colision detection    
    byte ColisionInvaderBeam(byte j) {
      // 
      return (Colision(
        m_invaders[j].m_x, m_invaders[j].m_y, Invader::THICKNESS, Invader::THICKNESS, 
        m_beam.m_x, m_beam.m_y, 2, 4) ? 
        100 : 
        0);
    };
    /// shield-beam colision detection    
    byte ColisionShieldBeam(byte j) {
      // 
      return (Colision(
        m_shields[j].m_x, m_shields[j].m_y, Shield::THICKNESS, Shield::THICKNESS, 
        m_beam.m_x, m_beam.m_y, 2, 4) ? 
        25 : 
        0);
    };
    /// shield-invaderbeam colision detection
    byte ColisionShieldInvaderBeam(byte j, byte i) {
      // 
      return (Colision(
        m_shields[j].m_x, m_shields[j].m_y, Shield::THICKNESS, Shield::THICKNESS, 
        m_invaderBeams[i].m_x, m_invaderBeams[i].m_y, 2, 4) ? 
        25 : 
        0);
    };
    /// invader-beam colision detection    
    byte ColisionShipInvaderBeam(byte i) {
      // 
      return (Colision(
        m_ship.m_x, m_ship.m_y + 4, Ship::THICKNESS, 4, 
        m_invaderBeams[i].m_x, m_invaderBeams[i].m_y, 2, 4) ? 
        100 : 
        0);
    };
    /// colision detection    
    bool Colision(
      signed char x1, signed char y1, signed char w1, signed char h1, 
      signed char x2, signed char y2, signed char w2, signed char h2) {
      // 
      if (x1 >= x2 + w2) {
        // 
        return false;
      }
      // 
      if (x2 >= x1 + w1) {
        // 
        return false;
      }
      // 
      if (y1 >= y2 + h2) {
        // 
        return false;
      }
      // 
      if (y2 >= y1 + h1) {
        // 
        return false;
      }
      // 
      return true;
    };
    
    /// char display
    void Char(char character, byte x, byte y, bool black) {
      // 
      unsigned char col;
      for (byte i = 0; i < 3; i ++) {
        // by column
        col = NUMBERS[character - 0x30][i];
        for (byte j = 0; j < 5; j ++) {
          // by row
          if (col & (0x01 << j)) {
            // 
            m_lcd.Point(x + i, y + j, black);
          } else {
            // 
            m_lcd.Point(x + i, y + j, !black);
          }
        }
      }
    }
    /// char pointer display
    void Text(const char * pString, byte x, byte y, bool black) {
      // 
      while (*pString != 0x00) {
        // loop until null terminator
        Char(*pString ++, x, y, black);
        x += 3;
        for (int i = y; i < y + 8; i ++) {
          // 
          m_lcd.Point(x, i, !black);
        }
        x ++;
        if (x > (WIDTH - 3)) {
          // enables wrap around
          x = 0;
          y += 5;
        }
      }
    }
    /// string display
    void Text(String s, byte x, byte y, bool black) {
      // 
      return Text(s.c_str(), x, y, black);
    }
    
    /// the state of the automate
    unsigned char m_state;
    /// invader speed vector
    signed char m_invaderSpeed;
    /// time divider
    unsigned char m_timeDivider;
    /// Joystick position on X axis
    signed char m_joysickX;
    /// high-res time probes
    unsigned long m_nowTime, m_lastTime;
    /// the ship
    Ship m_ship;
    /// shields
    Shield m_shields[3];
    /// invaders
    Invader m_invaders[16];
    /// laser beams
    LaserBeam m_beam, m_invaderBeams[2];
    /// invader laser beam index
    unsigned char m_invaderLaserIndex;
    /// colision degree
    unsigned char m_colisionDegree;
    /// score
    unsigned int m_score;
    /// lives
    signed char m_lives, m_stepDown;
    /// random sequence index
    unsigned char m_level, m_randomIndex, m_invaderCount, m_invaderAdvancePeriod;
    /// maneuver string object
    String m_tempStr;
};

const unsigned char SpaceInvaders::RANDOM[16] = {
  143, 194, 173, 133, 125, 7, 68, 222, 
  174, 72, 195, 246, 46, 105, 86, 192
};

/// the game instance
SpaceInvaders game;

void setup() {
  // put your setup code here, to run once:
  game.Setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  game.Loop();
}
