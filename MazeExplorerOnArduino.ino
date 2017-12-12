/*
 * Maze Explorer for the Arduino Game Console
 * https://www.gameinstance.com/post/51/Maze-Explorer-game
 *
 * GameInstance.com
 * 2017
 */

#include <GameConsole.h>

struct Point {
  
  unsigned char m_x, m_y;

  void Init(unsigned char x, unsigned char y) {
    //
    m_x = x;
    m_y = y;
  }
};

struct Maze {

  // can travel up
  bool UpBlocked(unsigned int i) {
    // 
    if (PLAIN) {
      // 
      return (m_map[i] & 0x01);
    }
    return (m_map[i / 4] & (0x01 << ((3 - (i % 4)) * 2)));
  }
  // can travel left
  bool LeftBlocked(unsigned int i) {
    // 
    if (PLAIN) {
      // 
      return (m_map[i] & 0x02);
    }
    return (m_map[i / 4] & (0x02 << ((3 - (i % 4)) * 2)));
  }
/*
  /// maze size
  static const unsigned char W = 6, H = 4;
  /// the maze
  unsigned char m_map[(int)ceil(W * H / (PLAIN ? 1 : 4))] {
    0xF5, 0xDA, 0xCA, 0x89, 0x2D, 0x54
//    3, 3, 1, 1, 3, 1, 
//    2, 2, 3, 0, 2, 2, 
//    2, 0, 2, 1, 0, 2, 
//    3, 1, 1, 1, 1, 0
  };
  /// plain data representation
  static const bool PLAIN = false;
*/

  /// maze size
  static const unsigned char W = 9, H = 6;
  /// the maze
  unsigned char m_map[W * H] {
    3, 1, 1, 1, 1, 3, 1, 3, 1, //1, 1, 
    3, 1, 3, 1, 0, 2, 2, 2, 1, //3, 0,  
    3, 2, 2, 1, 1, 0, 2, 2, 3, //0, 3,
    2, 2, 1, 3, 1, 1, 0, 0, 3, //1, 0, 
    2, 0, 2, 2, 1, 3, 1, 2, 0, //3, 0, 
    3, 1, 0, 1, 1, 0, 2, 1, 0, //2, 1
  };
  /// plain data representation
  static const bool PLAIN = true;
};

struct Walker : public Point {
  
  static const byte TICKNESS = 6;
  static const unsigned char MASK[3];

  unsigned char m_direction;
  signed char m_dx, m_dy;

  Walker() : Point(), 
    m_direction(0), 
    m_dx(0), m_dy(0) {
    // 
  }
  
  /// still moving
  bool IsMoving() {
    // 
    return (m_direction != 0);
  }
  /// moves horizontally
  void GoX(signed char x, const Maze& maze) {
    // 
    if ((m_x + maze.H < 0) || (m_x + x > maze.W - 1)) {
      // 
      return;
    }
    if (x < 0) {
      // go left
      if (maze.LeftBlocked(m_x + m_y * maze.W)) {
        // left is blocked
        return;
      }
      m_direction = 1;
      return;
    }
    if (x > 0) {
      // go right
      if (maze.LeftBlocked(m_x + x + m_y * maze.W)) {
        // right is blocked
        return;
      }
      m_direction = 3;
      return;
    }
    return;
  }
  /// moves vertically
  bool GoY(signed char y, const Maze& maze) {
    // 
    if ((m_y + y < 0) || (m_y + y > maze.H - 1)) {
      // 
      return;
    }
    if (y < 0) {
      // go up
      if (maze.UpBlocked(m_x + m_y * maze.W)) {
        // up is blocked
        return;
      }
      m_direction = 2;
      return;
    }
    if (y > 0) {
      // go down
      if (maze.UpBlocked(m_x + (m_y + y) * maze.W)) {
        // down is blocked
        return;
      }
      m_direction = 4;
      return;
    }
    return;
  }
  /// moves the walker
  bool Move(unsigned char width, unsigned char height) {
    // 
    if (m_direction == 1) {
      // going left
      m_dx -= 1;
      if (m_dx <= -width) {
        // 
        m_dx = 0;
        m_x -= 1;
        m_direction = 0;
      }
    }
    if (m_direction == 2) {
      // going up
      m_dy -= 1;
      if (m_dy <= -height) {
        // 
        m_dy = 0;
        m_y -= 1;
        m_direction = 0;
      }
    }
    if (m_direction == 3) {
      // going right
      m_dx += 1;
      if (m_dx >= width) {
        // 
        m_dx = 0;
        m_x += 1;
        m_direction = 0;
      }
    }
    if (m_direction == 4) {
      // going down
      m_dy += 1;
      if (m_dy >= height) {
        // 
        m_dy = 0;
        m_y += 1;
        m_direction = 0;
      }
    }
  }
};

const unsigned char Walker::MASK[3] = {0x0E, 0x15, 0x17};

class MazeExplorer : public GameConsole {

  public:
    /// default constructor
    MazeExplorer() : GameConsole(), 
      m_state(0), 
      m_nowTime(0), 
      m_lastTime(0) {
      //
    };
    /// destructor
    virtual ~MazeExplorer() {
      //
    };
    /// the game index
    unsigned char GameIndex() {
      // 
      return 2;
    }
    /// game setup method
    void Setup() {
      // 
      GameConsole::Setup();
      m_lcd.Contrast(45);
      m_lcd.Fill(false);
      m_lcd.Text("MAZE", 31, 10, true);
      m_lcd.Text("EXPLORER", 19, 22, true);
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
      
      if (m_state == 0) { // initial game state
        // 
        m_lastTime = m_nowTime;
        m_walker.Init(0, 0);
        m_final.Init(0, 3);
        m_state = 1;
      }

      if (m_state == 1) { // reading input
        // 
        m_joystickX = GetAxis(AXIS_X) / 64;
        m_joystickY = GetAxis(AXIS_Y) / 64;
        m_state = 2;
      }

      if (m_state == 2) { // moving
        // 
        if (m_walker.IsMoving()) {
          // 
          m_walker.Move(CHANNEL_WIDTH, CHANNEL_HEIGHT);
        }
        if (!m_walker.IsMoving()) {
          // 
          if (abs(m_joystickX) > abs(m_joystickY)) {
            // 
            if (m_joystickX > 0) {
              // right
              m_walker.GoX(1, m_maze);
            } else if (m_joystickX < 0) {
              // left
              m_walker.GoX(-1, m_maze);
            }
          } else {
            // 
            if (m_joystickY > 0) {
              // up
              m_walker.GoY(-1, m_maze);
            } else if (m_joystickY < 0) {
              // down
              m_walker.GoY(1, m_maze);
            }
          }
        }
        m_state = 3;
      }
      
      if (m_state == 3) { // display
        // clear screen
//        m_lcd.Fill(false);
        ClearDisplay();
        // maze
        for (unsigned char i = 0; i < m_maze.W * m_maze.H; i ++) {
          // 
          if (m_maze.UpBlocked(i)) {
            // top
            m_lcd.Line(
              SCREEN_OFFSET_X + x + (i % m_maze.W) * CHANNEL_WIDTH - dx, 
              SCREEN_OFFSET_Y + y + (i / m_maze.W) * CHANNEL_HEIGHT - dy, 
              SCREEN_OFFSET_X + x + (i % m_maze.W) * CHANNEL_WIDTH + dx, 
              SCREEN_OFFSET_Y + y + (i / m_maze.W) * CHANNEL_HEIGHT - dy, 
              true);
          }
          if (m_maze.LeftBlocked(i)) {
            // left
            m_lcd.Line(
              SCREEN_OFFSET_X + x + (i % m_maze.W) * CHANNEL_WIDTH - dx, 
              SCREEN_OFFSET_Y + y + (i / m_maze.W) * CHANNEL_HEIGHT - dy, 
              SCREEN_OFFSET_X + x + (i % m_maze.W) * CHANNEL_WIDTH - dx, 
              SCREEN_OFFSET_Y + y + (i / m_maze.W) * CHANNEL_HEIGHT + dy, 
              true);
          }
          if (i % m_maze.W == m_maze.W - 1) {
            // right - end
            m_lcd.Line(
              SCREEN_OFFSET_X + x + (i % m_maze.W) * CHANNEL_WIDTH + dx + 1, 
              SCREEN_OFFSET_Y + y + (i / m_maze.W) * CHANNEL_HEIGHT - dy, 
              SCREEN_OFFSET_X + x + (i % m_maze.W) * CHANNEL_WIDTH + dx + 1, 
              SCREEN_OFFSET_Y + y + (i / m_maze.W) * CHANNEL_HEIGHT + dy, 
              true);
          }
          if (i / m_maze.W == m_maze.H - 1) {
            // bottom - end
            m_lcd.Line(
              SCREEN_OFFSET_X + x + (i % m_maze.W) * CHANNEL_WIDTH - dx, 
              SCREEN_OFFSET_Y + y + (i / m_maze.W) * CHANNEL_HEIGHT + dy, 
              SCREEN_OFFSET_X + x + (i % m_maze.W) * CHANNEL_WIDTH + dx, 
              SCREEN_OFFSET_Y + y + (i / m_maze.W) * CHANNEL_HEIGHT + dy, 
              true);
          }
        }
        
        // walker
        unsigned char column;
        for (int i = 0; i < Walker::TICKNESS; i ++) {
          // by columns
          column = Walker::MASK[(i < 3) ? i : Walker::TICKNESS - i - 1];
          for (int j = 0; j < Walker::TICKNESS - 1; j ++) {
            // by rows
            if (column & (0x01 << j)) {
              // 
              m_lcd.Point(
                SCREEN_OFFSET_X + m_walker.m_x * CHANNEL_WIDTH + m_walker.m_dx + i + 2, 
                SCREEN_OFFSET_Y + m_walker.m_y * CHANNEL_HEIGHT + m_walker.m_dy + j + 2, 
                true);
            } else {
              // 
              m_lcd.Point(
                SCREEN_OFFSET_X + m_walker.m_x * CHANNEL_WIDTH + m_walker.m_dx + i + 2, 
                SCREEN_OFFSET_Y + m_walker.m_y * CHANNEL_HEIGHT + m_walker.m_dy + j + 2, 
                false);
            }
          }
        }

        // final point
        m_lcd.Point(
          SCREEN_OFFSET_X + m_final.m_x * CHANNEL_WIDTH + CHANNEL_WIDTH / 2, 
          SCREEN_OFFSET_Y + m_final.m_y * CHANNEL_HEIGHT + CHANNEL_HEIGHT / 2, 
          true);
        m_lcd.Update();
        m_state = 4;
      }

      if (m_state == 4) { // destination test
        // 
        if ((m_walker.m_x == m_final.m_x) 
          && (m_walker.m_y == m_final.m_y)) {
          // 
          m_state = 6;
        } else {
          // 
          m_state = 5;
        }        
      }
      
      if (m_state == 5) { // waiting state
        // 
        if (m_nowTime > m_lastTime + 40) {
          // 
          m_lastTime = m_nowTime;
          m_state = 1;
        }        
      }

      if (m_state == 6) { // game complete
        // 
        m_lcd.Text("Congrats!", 16, 4, true);
        m_lcd.Update();
        m_state = 7;
      }

      if (m_state == 7) { // end state
        // 
      }
    }


  private:

    /// on-screen maze offset
    unsigned char SCREEN_OFFSET_X = 1, SCREEN_OFFSET_Y = 0;
    /// maze channel dimensions
    static const unsigned char CHANNEL_WIDTH = 9, CHANNEL_HEIGHT = 8;

    /// the state of the automate
    byte m_state;
    /// high-res time probes
    unsigned long m_nowTime, m_lastTime;
    /// time divider
    unsigned char m_timeDivider;
    /// the walking character
    Walker m_walker;
    /// the maze
    Maze m_maze;
    /// the final destination
    Point m_final;
    /// alignment variables
    unsigned char dx = CHANNEL_WIDTH / 2, dy = CHANNEL_HEIGHT / 2, x = dx, y = dy;
    /// joystick position
    signed char m_joystickX, m_joystickY;
};

/// the game instance
MazeExplorer game;

void setup() {
  // put your setup code here, to run once:
  game.Setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  game.Loop();
}
