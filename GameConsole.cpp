#include <Arduino.h>
#include <EEPROM.h>
#include <GameConsole.h>


Storage::Storage() : m_gameIndex(0) {
    // 
}

Storage::~Storage() {
	// 
}

void Storage::Init(unsigned char gameIndex) {
    // 
    m_gameIndex = gameIndex;
}

unsigned int Storage::GetScore() {
    // 
    if (m_gameIndex == 0) {
        // 
        return 0;
    }
    // 
    unsigned char highByte = EEPROM.read(m_gameIndex * STORAGE_CHUNK_SIZE + 0);
    unsigned char lowByte = EEPROM.read(m_gameIndex * STORAGE_CHUNK_SIZE + 1);
    return (highByte << 8) + lowByte;
}

void Storage::SetScore(unsigned int value) {
    // 
    if (m_gameIndex == 0) {
        // 
        return;
    }
    // 
    unsigned char lowByte = (value & 0xFF);
    unsigned char highByte = ((value >> 8) & 0xFF);
    EEPROM.write(m_gameIndex * STORAGE_CHUNK_SIZE + 0, highByte);
    EEPROM.write(m_gameIndex * STORAGE_CHUNK_SIZE + 1, lowByte);
}

GameConsole::GameConsole() {
	// 
	m_centerX = m_centerY = 0;
}

GameConsole::~GameConsole() {
	// 
}

unsigned char GameConsole::GameIndex() {
	// 
	return 0;
}

void GameConsole::Setup() {
	// 
	for (unsigned char i = 0; i < BUTTON_COUNT; i ++) {
		// 
		m_bPress[i] = false;
	}
	/// CE, RST, DC, DIN, CLK, LED
	m_lcd.Config(7, 6, 8, 11, 13, 9);
	m_lcd.Start();
	m_lcd.Contrast(45);
	m_lcd.Light();
	m_lcd.Fill(false);
	m_lcd.Text("GameInstance", 8, 8, true);
	m_lcd.Text(".com", 30, 16, true);
	m_lcd.Text("Console 1.0", 12, 32, true);
	m_lcd.Update();
	m_centerX = analogRead(A2);
	m_centerY = analogRead(A1);
        m_storage.Init(GameIndex());
	delay(1000);
}

void GameConsole::ClearDisplay() {
	//
	m_lcd.Fill(false);
	m_lcd.Line(0, 0, WIDTH - 1, 0, true);
	m_lcd.Line(0, 0, 0, HEIGHT - 1, true);
	m_lcd.Line(WIDTH - 1, HEIGHT - 1, WIDTH - 1, 0, true);
	m_lcd.Line(WIDTH - 1, HEIGHT - 1, 0, HEIGHT - 1, true);
}

bool GameConsole::WasPressed(unsigned char button) {
	//
	unsigned char index = ButtonIndex(button);
	if (HIGH == digitalRead(button)) {
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

bool GameConsole::IsPressed(unsigned char button) {
	// 
	return (LOW == digitalRead(button));
}

int GameConsole::GetAxis(unsigned char axis) {
	// 
	return analogRead(axis) - (AXIS_X == axis ? m_centerX : m_centerY);
}

void GameConsole::Loop() {
    // 
    Execute();
}

unsigned char GameConsole::ButtonIndex(unsigned char button) {
	//
	switch (button) {
		// 
		case BUTTON_A: return 0;
		case BUTTON_B: return 1;
		case BUTTON_C: return 2;
	}
	// unknown button
	return 255;
}
