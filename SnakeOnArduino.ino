/*
 * The Snake game for Arduino
 * 
 *  GameInstance.com
 *  2016
 */

#include <SPI.h>
#include <LCDNokia5100.h>


/*
 * Snake block 
 * 
 * An element of the snake body
 */
class Block {

  public:

    /// default constructor
    Block() {
      //
      m_x = 0;
      m_y = 0;
      m_d = 0;
    };
    /// parametric constructor
    Block(byte x, byte y, byte d) {
      //
      m_x = x;
      m_y = y;
      m_d = d;
    };
    /// destructor
    virtual ~Block() {};

    /// the block coordinates
    byte m_x, m_y;
    /// the block's direction [1-4] with 0 meaning null 
    byte m_d;
};

/*
 * Snake food item
 * 
 */
class Food {

  public:

    /// default constructor
    Food() {
      //
      m_x = m_y = m_index = 0;
    };
    /// destructor
    virtual ~Food() {};

    /// generates random x,y coordinates for this
    /// not really working unless a noisy analog input is used for seeding
    void Random(byte width, byte height, byte weight) {
      //
      m_x = weight * random((width - 2) / weight) + 1;
      m_y = weight * random((height - 2) / weight) + 1;
    };
    /// generates the next x,y pair from a predetermined array of ints
    bool Next(byte width, byte height, byte weight) {
      //
      m_x = weight * (m_Points[m_index] % ((width - 2) / weight)) + 1;
      m_y = weight * (m_Points[m_index] % ((height - 2) / weight)) + 1;
      m_index ++;
      if (m_index >= 10 - 1) {
        //
        m_index = 0;
        return false;
      }
      return true;
    };
    /// draws this onto a given LCD display
    void Draw(LCDNokia5100 &lcd, byte weight) {
      // 
      lcd.Rect(m_x, m_y, m_x + weight, m_y + weight - 1, true, true);
    };

    /// the food coordinates 
    byte m_x, m_y;
    /// the index in the array
    byte m_index;
    /// a predetermined array used for generating on-screen coordinates
    unsigned int m_Points[10] = {1301, 2392, 4812, 9831, 6127, 9928, 3123, 4123, 6512, 4812};
};


/*
 * The snake
 * 
 */
class Snake {

  public:

    /// the maximum length of the snake
    static const byte BLOCK_COUNT_MAX = 20;

    /// default constructor
    Snake() {
      // 
      m_Size = 0;
      m_HeadX = 31;
      m_HeadY = 31;
      m_Speed = 1;
      m_Direction = 2;
      m_Weight = 1;
      m_bSelfTouch = false;
      memset(m_Blocks, 0, BLOCK_COUNT_MAX * sizeof(Block));
    };
    /// destructor
    virtual ~Snake() {
      //
      for (byte i = 0; i < m_Size; i ++) {
        //
        delete m_Blocks[i];
      }
    };

    /// sets the weight of the snake
    void SetWeight(byte w) {
      //
      m_Weight = w;
    };
    /// adds a block at the end of the snake
    void AddBlock() {
      //
      byte dx, dy;
      if (m_Size == 0) {
        // first block to be added
        m_Blocks[m_Size] = new Block(m_HeadX, m_HeadY, m_Direction);
      } else {
        // not the first block added
        GetDirection(dx, dy, m_Blocks[m_Size - 1]->m_d);
        m_Blocks[m_Size] = new Block(m_Blocks[m_Size - 1]->m_x + dx * m_Weight, m_Blocks[m_Size - 1]->m_y + dy * m_Weight, m_Blocks[m_Size - 1]->m_d);
      }
      m_Size ++;
    };
    /// executes the slithering movement
    void Execute(byte dt = 1) {
      //
      byte dx, dy;
      for (byte i = 0; i < m_Size; i ++) {
        //
        GetDirection(dx, dy, m_Blocks[i]->m_d);
        m_Blocks[i]->m_x -= m_Speed * dt * dx * m_Weight;
        m_Blocks[i]->m_y -= m_Speed * dt * dy * m_Weight;
      }
      m_HeadX = m_Blocks[0]->m_x;
      m_HeadY = m_Blocks[0]->m_y;
      for (byte i = m_Size - 1; i > 0; i --) {
        // 
        m_Blocks[i]->m_d = m_Blocks[i - 1]->m_d;
        if ((m_HeadX == m_Blocks[i]->m_x) && (m_HeadY == m_Blocks[i]->m_y)) {
          // snake head touches own body
          m_bSelfTouch = true;
        }
      }
      m_Blocks[0]->m_d = m_Direction;
    };
    /// sets the snake's direction
    void SetDirection(byte d) {
      // 
      if (m_Direction == d) {
        // no direction change
        return;
      }
      // direction has changed
      m_Direction = d;
    };
    /// turns the snake left or right
    void Turn(byte right = 1) {
      //
      byte d = m_Direction;
      d += right;
      if (d > 4) {
        // 
        d = 1;
      } else if (d < 1) {
        // 
        d = 4;
      }
      SetDirection(d);
    };
    /// draws the snake onto the LCD
    void Draw(LCDNokia5100 &lcd) {
      // 
      for (byte i = 0; i < m_Size; i ++) {
        //
        lcd.Rect(m_Blocks[i]->m_x, m_Blocks[i]->m_y, m_Blocks[i]->m_x + m_Weight, m_Blocks[i]->m_y + m_Weight - 1, true, true);
      }
    };
    

    /// speed of the snake and direction of the head
    byte m_Speed, m_Direction;
    /// the length of the snake and the size of the block element
    byte m_Size, m_Weight;
    /// snake's head coordinates
    byte m_HeadX, m_HeadY;
    /// head touching body indicator
    bool m_bSelfTouch;


  private:

    /// gets the (x, y) direction given the modeled direction
    void GetDirection(byte &dx, byte &dy, byte d) {
      //
      dx = (d == 2) ? 1 : ((d == 4) ? -1 : 0);
      dy = (d == 3) ? 1 : ((d == 1) ? -1 : 0);
    };

    /// the array of blocks forming the snake boby
    Block* m_Blocks[BLOCK_COUNT_MAX];
};

/*
 * The main game class
 * 
 */
class Game {

  public:

    /// the display width in pixels
    static const byte WIDTH = 84;
    /// the display height in pixels
    static const byte HEIGHT = 48;
    /// the number of buttons in use
    static const byte BUTTON_COUNT = 2;
    /// the size of the snake block
    static const byte WEIGHT = 2;

    /// default constructor
    Game() {
      //
      m_state = 0;
    };
    /// destructor
    virtual ~Game() {
      //
    };

    /// generates an arbitrary game configuration
    void Setup() {
      // 
      m_s.SetWeight(WEIGHT);
      for (byte i = 0; i < 9; i ++) {
        // 
        m_s.AddBlock();
      }
      //randomSeed(analogRead(A5));
      m_f.Next(WIDTH, HEIGHT, WEIGHT);
      for (byte i = 0; i < BUTTON_COUNT; i ++) {
        // 
        m_bPress[i] = false;
      }
      // Setting the LCD config as described on 
      // https://www.gameinstance.com/post/15/The-Snake-game-for-Arduino
      m_lcd.Config(9, 12, 8, 11, 13, 10);
      // 
      m_lcd.Start();
      m_lcd.Contrast(45);
      m_lcd.Light();
      m_lcd.Fill(false);
      m_lcd.Text("GameInstance", 8, 8, true);
      m_lcd.Text(".com", 30, 16, true);
      m_lcd.Text("The Snake", 12, 32, true);
      m_lcd.Update();
      m_delay = 200;
      delay(1000);
    };
    /// the game state machine
    void Execute() {
      // 
      if (m_state == 0) {
        // game start
        m_state = 1;
      }
      if (m_state == 1) {
        // game on
        if (WasPressed(A0, 0)) {
          // 
          m_s.Turn(1);
        }
        if (WasPressed(A1, 1)) {
          // 
          m_s.Turn(-1);
        }
        m_s.Execute();
        Draw();
        if ((m_s.m_HeadX == m_f.m_x) 
          && (m_s.m_HeadY == m_f.m_y)) {
          // 
          m_state = 2;
        } else {
          //
          if ((m_s.m_HeadX < 1)
            || (m_s.m_HeadX > WIDTH - 2)
            || (m_s.m_HeadY < 1)
            || (m_s.m_HeadY > HEIGHT - 2)) {
            // 
            m_state = 10;
          }
          if (m_s.m_bSelfTouch) {
            //
            m_state = 10;
          }
        }
      }
      if (m_state == 2) {
        // food eaten
        m_s.AddBlock();
        if (!m_f.Next(WIDTH, HEIGHT, WEIGHT)) {
          // 
          m_state = 5;
        } else {
          //
          m_state = 1;
        }
      }
      if (m_state == 5) {
        // game complete
        m_lcd.Fill(false);
        m_lcd.Text("Congrats!", 14, 8, true);
        m_lcd.Text("press reset", 8, 32, true);
        m_lcd.Update();
        m_state = 11;
      }
      if (m_state == 10) {
        // game over
        m_lcd.Fill(false);
        m_lcd.Text("Game Over!", 12, 8, true);
        m_lcd.Text("press reset", 8, 32, true);
        m_lcd.Update();
        m_state = 11;
      }
      if (m_state == 11) {
        // end
      }
    };
    /// draws the game components
    void Draw() {
      //
      m_lcd.Fill(false);
      m_lcd.Line(0, 0, WIDTH - 1, 0, true);
      m_lcd.Line(0, 0, 0, HEIGHT - 1, true);
      m_lcd.Line(WIDTH - 1, HEIGHT - 1, WIDTH - 1, 0, true);
      m_lcd.Line(WIDTH - 1, HEIGHT - 1, 0, HEIGHT - 1, true);
      m_s.Draw(m_lcd);
      m_f.Draw(m_lcd, WEIGHT);
      m_lcd.Update();
    };
    
    int m_delay;


  private:

    /// indicates once that a button was pressed
    bool WasPressed(byte pin, byte index, int threshold = 512) {
      //
      int val = analogRead(pin);
      //Serial.println(val);
      if (val > threshold) {
        // isn't pressed
        if (m_bPress[index]) {
          // but was before
          m_bPress[index] = false;
        }
        return false;
      }
      // is pressed
      if (!m_bPress[index]) {
        // and wasn't before
        m_bPress[index] = true;
        return true;
      }
      // but was before
      return false;
    }

    /// the state of the automate
    byte m_state;
    /// the snake
    Snake m_s;
    /// the food
    Food m_f;
    /// the display
    LCDNokia5100 m_lcd;
    /// the array of button states
    bool m_bPress[BUTTON_COUNT];
};


/// the game instace
Game g;

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  g.Setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  g.Execute();
  delay(g.m_delay);
}
