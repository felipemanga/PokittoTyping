template<typename UI, typename INPUT> class Keyboard;

#define BTN_D BTN_C

class KBWriter;

void writeStr( KBWriter &writer, const char *label );

struct KBState {

    const char *label;
    const KBState *states;
    void (*action)( KBWriter &, const char *label );

};

#define KBSTATE( letters ) const KBState state ## letters[8] =

#define WRITE( str ) { #str , nullptr, writeStr }
#define WRITEMOD( str, mod ) { #str , state ## mod, writeStr }

#define SKIPDIR() {}, {}, {}, {}

#define KBSTATE3(A, B, C, D, E)			\
    KBSTATE( A ## B ## C ){			\
	SKIPDIR(),				\
	    WRITE(A), WRITE(B), WRITE(C)	\
    };						\
    KBSTATE( B ## C ## D ){			\
	SKIPDIR(),				\
	    WRITE(B), WRITE(C), WRITE(D),	\
    };						\
    KBSTATE( C ## D ## E ){			\
	SKIPDIR(),				\
	    WRITE(C), WRITE(D), WRITE(E),	\
    };

#define KBSTATE5(A, B, C, D, E)				  \
    KBSTATE3( A, B, C, D, E )				  \
    KBSTATE( A ## B ## C ## D ## E ){			  \
	SKIPDIR(),					  \
	    WRITEMOD( A, A ## B ## C ),			  \
	    WRITEMOD( C, B ## C ## D ),			  \
	    WRITEMOD( E, C ## D ## E ),			  \
	}

KBSTATE5(F,G,H,I,J);
KBSTATE5(P,Q,R,S,T);
KBSTATE5(0,1,2,3,4);

KBSTATE( Zdc ){		
    SKIPDIR(),			
    {},
    {"'", nullptr, writeStr},
};

KBSTATE( dcq ){		
    SKIPDIR(),			
    {"'", nullptr, writeStr},
    {",", nullptr, writeStr},
    {"?", nullptr, writeStr},	
};

KBSTATE( cqe ){		
    SKIPDIR(),			
    {",", nullptr, writeStr},
    {"?", nullptr, writeStr},
    {"!", nullptr, writeStr},	
};

KBSTATE( Z_ ){
    SKIPDIR(),
    WRITEMOD( Z, Zdc ),
    { "," , statedcq, writeStr },
    { "!" , statecqe, writeStr },
};

KBSTATE3(A, B, C, D, E)
KBSTATE(ABCDE){
    {},
    {"FGHIJ", stateFGHIJ},
    {},
    {"01234", state01234},
    WRITEMOD(A, ABC),
    WRITEMOD(C, BCD),
    WRITEMOD(E, CDE),
};

KBSTATE3( K, L, M, N, O )
KBSTATE(KLMNO){
    {"FGHIJ", stateFGHIJ},
    {},
    {"PQRST", statePQRST},
    {},
    WRITEMOD(K, KLM),
    WRITEMOD(M, LMN),
    WRITEMOD(O, MNO),
};


KBSTATE3( U, V, W, X, Y )
KBSTATE(UVWXY){
    {},
    {"PQRST", statePQRST},
    {},
    {"Z',?!", stateZ_},
    WRITEMOD(U, UVW),
    WRITEMOD(W, VWX),
    WRITEMOD(Y, WXY),
};


KBSTATE3( 5, 6, 7, 8, 9 )
KBSTATE(56789){
    {"01234", state01234},
    {},
    {"Z',?!", stateZ_},
    {},
    WRITEMOD(5, 567),
    WRITEMOD(7, 678),
    WRITEMOD(9, 789),
};

KBSTATE(ROOT){
    {"ABCDE", stateABCDE},
    {"KLMNO", stateKLMNO},
    {"UVWXY", stateUVWXY},
    {"56789", state56789}
};

#undef WRITE
#undef KBSTATE
#undef SKIPDIR

class KBInput {
    
    template<typename UI, typename INPUT> friend class Btnboard;

    uint8_t sequence[9], prevBtnState, btnState, badSequence;
    
    bool isPressedA(){ return ((volatile uint8_t *) 0xA0000020)[9]; }
    bool isPressedB(){ return ((volatile uint8_t *) 0xA0000020)[4]; }
    bool isPressedC(){ return ((volatile uint8_t *) 0xA0000020)[10]; }
    bool isPressedD(){ return ((volatile uint8_t *) 0xA0000000)[1]; }
    bool isPressedUp(){ return ((volatile uint8_t *) 0xA0000020)[13]; }
    bool isPressedDown(){ return ((volatile uint8_t *) 0xA0000020)[3]; }
    bool isPressedLeft(){ return ((volatile uint8_t *) 0xA0000020)[25]; }
    bool isPressedRight(){ return ((volatile uint8_t *) 0xA0000020)[7]; }

    void pollButtons(){
	btnState = 0;
	btnState |= isPressedA() ? 1 << BTN_C : 0;
	btnState |= isPressedB() ? 1 << BTN_B : 0;
	btnState |= isPressedC() ? 1 << BTN_A : 0;
	btnState |= isPressedUp() ? 1 << BTN_UP : 0;
	btnState |= isPressedDown() ? 1 << BTN_DOWN : 0;
	btnState |= isPressedLeft() ? 1 << BTN_LEFT : 0;
	btnState |= isPressedRight() ? 1 << BTN_RIGHT : 0;

	if( !btnState )
	    badSequence = false;

	
	// Pokitto::Display::setCursor(0,10);
	// Pokitto::Display::print( uint32_t(btnState) );
	
    }

    void internalBtnPressed( uint8_t btn ){

	for( uint32_t i=0; i<8; ++i ){

	    if( sequence[i] == btn )
		return; // not supposed to happen

	    if( sequence[i] == 0xFF ){
		sequence[i] = btn;
		sequence[i+1] = 0xFF;
		updateSequence( sequence, false );
		return;
	    }

	}

    }

    void internalBtnReleased( uint8_t btn ){

	for( uint32_t i=0; i<8; ++i ){
	    
	    if( sequence[i] == btn ){
		
		if( sequence[i+1] != 0xFF ){

		    for( ; i<8; ++i )
			sequence[i] = 0xFF;
		    
		}

		updateSequence(sequence, true);
		sequence[i] = 0xFF;

		return;
		
	    }
	    
	}

    }

public:

    KBInput(){
	resetSequence();
    }

    virtual void updateSequence( uint8_t *sequence, uint8_t released );

    void resetSequence(){
	sequence[0] = 0xFF;
	prevBtnState = 0;
	badSequence = true;
    }

    void update(){
	pollButtons();

	if( badSequence )
	    return;

	uint32_t btnDiff = btnState ^ prevBtnState;
	prevBtnState = btnState;

	uint32_t btn = 0;
	while( btnDiff ){

	    if( btnDiff&1 ){
		if( btnState & (1<<btn) )
		    internalBtnPressed( btn );
		else
		    internalBtnReleased( btn );
	    }

	    btn++;
	    btnDiff>>=1;

	}
	
    }
    
};

class KBWriter {
public:
    virtual void key( uint8_t key );
};

class KBWriterBuffer : public KBWriter {

    uint32_t pos;
    char *buffer;
    size_t length;
    bool done;

    virtual void key( uint8_t key ){
	if( key == '\n' ){
	    done = true;
	}else if( key == 8 ){
	    if( pos == 0 ) return;
	    pos--;
	}else if( pos < length-1 && !done ){
	    buffer[ pos++ ] = key;
	    buffer[ pos ] = 0;
	    // printf("Str:[%s]\n", buffer);
	}
    }

public:
    void clear(){
	buffer[0] = 0;
	pos = 0;
	done = false;
    }

    template<size_t LEN> void setBuffer( char (&buf)[LEN] ){
	length = LEN;
	buffer = buf;
	pos = 0;
	done = false;
    }
    
};

class KBUIDirect {

    void enable(){
	// to-do: send this as a PR
	
	Pokitto::Display::enableDirectPrinting(true);
	// write_command_16(0x03);
	CLR_CS; // select lcd
	CLR_CD; // clear CD = command
	SET_RD; // RD high, do not read
   
	// setup_data_16(data); // function that inputs the data into the relevant bus lines
	LPC_GPIO_PORT->MPIN[2] = (0x03<<3); // write bits to port
   
	CLR_WR_SLOW;  // WR low
	SET_WR;  // WR low, then high = write strobe
	SET_CS; // de-select lcd
    
	// write_data_16(0x1030);
	CLR_CS;
	SET_CD;
	SET_RD;
	// setup_data_16(data);
	LPC_GPIO_PORT->MPIN[2] = (0x1030<<3); // write bits to port
	CLR_WR;
	SET_WR;
	SET_CS;    
    }

public:

    static int16_t bgColor,
	fontColor;

    void update(){

	
	
    }
    
};

struct Coord {
    uint8_t x, y, alignment;
};

const Coord mode2Coords[] = {
    {   0, 55, 0 },
    {  55, 35, 1 },
    { 110, 55, 2 },
    {  55, 75, 1 },

    { 30, 75, 1 },
    { 50, 55, 1 },
    { 70, 35, 1 },
    { 90, 15, 1 },
    
};

class KBUIMode2 {

    static const uint32_t width = POK_LCD_W/2;
    static const uint32_t height = POK_LCD_H/2;

void vline( uint32_t x, uint32_t y, uint32_t h, uint32_t color ){
    if( x >= width || y >= height || !h ) return;
    if( y+h >= height )
	h = height - y;

    uint8_t *fb = Pokitto::Display::screenbuffer + y * (width>>1) + (x>>1);
    
    color &= 0xF;
    uint8_t mask = 0xF;

    if( x & 1 ){
	mask <<= 4;	
    }else{
	color <<= 4;
    }
    
    while( h-- ){
	*fb = (*fb & mask) | color;	    
	fb += (width>>1);
    }
}

void hline( uint32_t x, uint32_t y, uint32_t w, uint32_t color ){
    if( x >= width || y >= height || !w ) return;
    if( x+w >= width )
	w = width - x;

    uint8_t *fb = Pokitto::Display::screenbuffer + y * (width>>1) + (x>>1);
    if( x&1 ){
	*fb = (*fb&0xF0) | (color);
	fb++;
	w--;
    }

    if( w&1 ){
	fb[w>>1] = (fb[w>>1]&0xF) | (color<<4);
	w--;
    }

    color &= 0xF;
    color |= color << 4;
    w >>= 1;
    while( w-- ){
	*fb++ = color;
    }

}

void fillRect( int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t color ){
    if( x >= width || y >= height ) return;
    if( y < 0 ){
	h += y;
	y = 0;
    }
    if( x < 0 ){
	w += x;
	x = 0;
    }
    if( y+h >= height )
	h = height - y;
    if( x+w >= width )
	w = width - x;
    
    if( x&1 ){
	vline(x, y, h, color);
	x++;
	w--;
    }

    if( w&1 ){
	vline( x+w-1, y, h, color );
	w--;
    }

    color &= 0xF;
    color |= color << 4;
    w >>= 1;
    while( h-- ){
	uint8_t *fb = Pokitto::Display::screenbuffer;
	fb += (y++ * width>>1) + (x>>1);
	
	for( uint32_t i=0; i<w; ++i )
	    *fb++ = color;
    }
}

public:

    int32_t adjustAlignment( const char *label, uint32_t alignment ){
	using Pokitto::Display;
	
	if( !alignment )
	    return 0;

	int32_t w = 0;
	for( uint32_t j=0; label[j]; ++j )
	    w += Display::fontWidth;

	if( alignment == 1 )
	    return w >> 1;

	return w;
	
    }

    void update( const KBState *states ){
	using Pokitto::Display;

	fillRect( 0, 30, width, height, 9 );

	Display::bgcolor = Display::invisiblecolor;
	Display::color = 10;

	uint32_t start = 0, end = 4;

	if( states != stateROOT ){
	    start = 4; end = 8;
	}
	
	for( uint32_t i=start; i<end; ++i ){
	    const char *label = states[i].label;

	    if( !label )
		continue;

	    int32_t x = mode2Coords[i].x, y;

	    x -= adjustAlignment( label, mode2Coords[i].alignment );

	    Display::setCursor( x, mode2Coords[i].y );
	    Display::print( label );

	    uint32_t next = i + 1;
	    if( next == end ) next = start;
	    
	    if( i < 6 && states[i].states && states[i].states[next].label ){

		label = states[i].states[next].label;		
		x = (int32_t(mode2Coords[i].x) + int32_t(mode2Coords[next].x)) >> 1;
		x -= adjustAlignment( label, 1 );
		y = (int32_t(mode2Coords[i].y) + int32_t(mode2Coords[next].y)) >> 1;

		Display::setCursor( x, y );
		Display::print( label );
		
	    }

	}

    }
    
};

template<typename UI, typename WRITER> class Keyboard : KBInput {
public:

    UI ui;
    WRITER writer;
    const KBState *activeState = stateROOT;
    bool actionLocked = true;

    virtual void updateSequence(
	uint8_t *sequence,
	uint8_t released
	){

	if( !released )
	    actionLocked = false;

	activeState = stateROOT;
	for( uint32_t i=0; i<8; ++i ){
	    uint32_t stateNumber = sequence[ i ];
	    if( stateNumber == 0xFF )
		break;

	    auto &nextState = activeState[ stateNumber ];

	    if( nextState.states && (sequence[i+1] != 0xFF || !released) ){

		activeState = nextState.states;
		
	    }else if( nextState.action && sequence[i+1] == 0xFF ){
		
		if( released ){

		    if( actionLocked )
			break;

		    nextState.action( writer, nextState.label );
		    actionLocked = true;
		    break;
		}
		
	    }else{
		break;
	    }

	}
	
    }
    

    void update(){
	KBInput::update();
	ui.update( activeState );
    }

};

void writeStr( KBWriter &writer, const char *label ){
    while( *label ){
	writer.key( *label++ );
    }
}

