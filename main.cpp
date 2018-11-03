#include <Pokitto.h>
#include "keyboard.h"

const char *words[] = {
    "We're", "no", "strangers", "to", "love",
"You", "know", "the", "rules", "and", "so", "do", "I",
"A", "full", "commitment's", "what", "I'm", "thinking", "of",
"You", "wouldn't", "get", "this", "from", "any", "other", "guy",
"I", "just", "wanna", "tell", "you", "how", "I'm", "feeling",
"Gotta", "make", "you", "understand",
"Never", "gonna", "give", "you", "up",
"Never", "gonna", "let", "you", "down",
"Never", "gonna", "run", "around", "and", "desert", "you",
"Never", "gonna", "make", "you", "cry",
"Never", "gonna", "say", "goodbye",
"Never", "gonna", "tell", "a", "lie", "and", "hurt", "you",
"We've", "known", "each", "other", "for", "so", "long",
"Your", "heart's", "been", "aching", "but", "you're",
"too", "shy", "to", "say", "it",
"Inside", "we", "both", "know", "what's", "been", "going", "on",
"We", "know", "the", "game", "and", "we're", "gonna", "play", "it",
"And", "if", "you", "ask", "me", "how", "I'm", "feeling",
"Don't", "tell", "me", "you're", "too", "blind", "to", "see",
"Never", "gonna", "give", "you", "up",
"Never", "gonna", "let", "you", "down",
"Never", "gonna", "run", "around", "and", "desert", "you",
"Never", "gonna", "make", "you", "cry",
"Never", "gonna", "say", "goodbye",
"Never", "gonna", "tell", "a", "lie", "and", "hurt", "you",
"Never", "gonna", "give", "you", "up",
"Never", "gonna", "let", "you", "down",
"Never", "gonna", "run", "around", "and", "desert", "you",
"Never", "gonna", "make", "you", "cry",
"Never", "gonna", "say", "goodbye",
"Never", "gonna", "tell", "a", "lie", "and", "hurt", "you",
"Never", "gonna", "give,", "never", "gonna", "give",
"Give", "you", "up",
"Ooh", "Never", "gonna", "give,", "never", "gonna", "give",
"Give", "you", "up",
"We've", "known", "each", "other", "for", "so", "long",
"Your", "heart's", "been", "aching", "but", "you're", "too", "shy", "to", "say", "it",
"Inside", "we", "both", "know", "what's", "been", "going", "on",
"We", "know", "the", "game", "and", "we're", "gonna", "play", "it",
"I", "just", "wanna", "tell", "you", "how", "I'm", "feeling",
"Gotta", "make", "you", "understand",
"Never", "gonna", "give", "you", "up",
"Never", "gonna", "let", "you", "down",
"Never", "gonna", "run", "around", "and", "desert", "you",
"Never", "gonna", "make", "you", "cry",
"Never", "gonna", "say", "goodbye",
"Never", "gonna", "tell", "a", "lie", "and", "hurt", "you",
"Never", "gonna", "give", "you", "up",
"Never", "gonna", "let", "you", "down",
"Never", "gonna", "run", "around", "and", "desert", "you",
"Never", "gonna", "make", "you", "cry",
"Never", "gonna", "say", "goodbye",
"Never", "gonna", "tell", "a", "lie", "and", "hurt", "you",
"Never", "gonna", "give", "you", "up",
"Never", "gonna", "let", "you", "down",
"Never", "gonna", "run", "around", "and", "desert", "you",
"Never", "gonna", "make", "you", "cry",
nullptr
};

uint32_t nextWord,
    wordX,
    wordY = 20,
    letter,
    timeToMove,
    currtime;

void reset(){
    nextWord = 0;
    wordX = 100;
    letter = 0;
    timeToMove = 25;
    currtime = 1;
}

void gameOver(){
    using Pokitto::Display;

    Display::clear();
    Display::setCursor( 15, 40 );
    Display::print("Game Over!");

    Display::update();
    
    while( !Pokitto::Core::aBtn() )
	Pokitto::Buttons::pollButtons();
    while( Pokitto::Core::aBtn() )
	Pokitto::Buttons::pollButtons();

    reset();
}

int main () {
    using Pokitto::Display;

    Keyboard< KBUIMode2, KBWriterBuffer > kb;

    char buf[32];
    kb.writer.setBuffer( buf );

    Pokitto::Core::begin();
    // Display::persistence = 1;
    Display::textWrap = false;
    reset();

    char mistake;

    while( Pokitto::Core::isRunning() ){

        if( !Pokitto::Core::update() )
	    continue;

	Pokitto::Buttons::pollButtons();
	kb.update();

	char nextLetter = words[nextWord][letter];
	
	if( nextLetter >= 'a' && nextLetter <= 'z' )
	    nextLetter -= 'a' - 'A';

	if( buf[0] == nextLetter ){
	    mistake = 0;
	    currtime = timeToMove*2;
	    letter++;
	    if( words[nextWord][letter] == 0 ){
		letter = 0;
		nextWord++;
		wordX = 100;
		if( words[nextWord] == nullptr ){
		    nextWord = 0;
		}
	    }
	}else if( buf[0] ){
	    mistake = buf[0];
	    if( wordX < 10 ) wordX = 10;
	    wordX -= 10;
	}

	kb.writer.clear();

	currtime--;
	if( currtime == 0 ){
	    currtime = timeToMove;
	    if( timeToMove < 10 )
		timeToMove = 10;

	    wordX--;
	    if( int32_t(wordX) <= 0 ){
		gameOver();
		mistake = 0;
		continue;
	    }
	}

	int32_t x = wordX;
	for( int32_t i=0; words[nextWord][i]; ++i ){

	    if( i >= letter ){
		Display::color = 5;
	    }else{
		Display::color = 8;
	    }

	    Display::print_char(x, wordY, words[nextWord][i]);
	    x += Display::fontWidth;

	    if( x > Display::width )
		break;
	    
	}

	if( mistake ){
	    Display::setCursor(0,0);
	    Display::print( mistake );
	}

	Display::bgcolor = 11;
    }
}

