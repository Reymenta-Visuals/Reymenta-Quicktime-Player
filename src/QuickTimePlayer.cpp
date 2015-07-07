#include "cinder/app/AppBasic.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
// spout
#include "Spout.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class QuickTimePlayer : public AppBasic {
 public:
	void setup();

	void keyDown( KeyEvent event );
	void fileDrop( FileDropEvent event );

	void update();
	void draw();

	void loadMovieFile( const fs::path &path );

	gl::Texture				mFrameTexture, mInfoTexture;
	qtime::MovieGlRef		mMovie;
	// spout
	SpoutSender spoutsender;                    // Create a Spout sender object
	bool bInitialized;                          // true if a sender initializes OK
	bool bMemoryMode;                           // tells us if texture share compatible
	unsigned int g_Width, g_Height;             // size of the texture being sent out
	char SenderName[256];                       // sender name 
	gl::Texture spoutTexture;                   // Local Cinder texture used for sharing

};

void QuickTimePlayer::setup()
{
	/*fs::path moviePath = getOpenFilePath();
	if( ! moviePath.empty() )
		loadMovieFile( moviePath );*/
	// spout
	g_Width = 640;
	g_Height = 480;
	// Set up the texture we will use to send out
	// We grab the screen so it has to be the same size
	spoutTexture = gl::Texture(g_Width, g_Height);
	strcpy_s(SenderName, "QuickTimePlayer"); // we have to set a sender name first
	// Initialize a sender
	bInitialized = spoutsender.CreateSender(SenderName, g_Width, g_Height);

}

void QuickTimePlayer::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f' ) {
		setFullScreen( ! isFullScreen() );
	}
	else if( event.getChar() == 'o' ) {
		fs::path moviePath = getOpenFilePath();
		if( ! moviePath.empty() )
			loadMovieFile( moviePath );
	}
	else if( event.getChar() == '1' )
		mMovie->setRate( 0.5f );
	else if( event.getChar() == '2' )
		mMovie->setRate( 2 );
}

void QuickTimePlayer::loadMovieFile( const fs::path &moviePath )
{
	try {
		// load up the movie, set it to loop, and begin playing
		mMovie = qtime::MovieGl::create( moviePath );
		mMovie->setLoop();
		mMovie->play();
		
		// create a texture for showing some info about the movie
		TextLayout infoText;
		infoText.clear( ColorA( 0.2f, 0.2f, 0.2f, 0.5f ) );
		infoText.setColor( Color::white() );
		infoText.addCenteredLine( moviePath.filename().string() );
		infoText.addLine( toString( mMovie->getWidth() ) + " x " + toString( mMovie->getHeight() ) + " pixels" );
		infoText.addLine( toString( mMovie->getDuration() ) + " seconds" );
		infoText.addLine( toString( mMovie->getNumFrames() ) + " frames" );
		infoText.addLine( toString( mMovie->getFramerate() ) + " fps" );
		infoText.setBorder( 4, 2 );
		mInfoTexture = gl::Texture( infoText.render( true ) );
	}
	catch (const std::exception &e) {
		console() << "Unable to load the movie:" << e.what() << std::endl;
		//mMovie->reset();
		mInfoTexture.reset();
	}

	mFrameTexture.reset();
}

void QuickTimePlayer::fileDrop( FileDropEvent event )
{
	loadMovieFile( event.getFile( 0 ) );
}

void QuickTimePlayer::update()
{
	if( mMovie )
		mFrameTexture = mMovie->getTexture();

	getWindow()->setTitle("(" + toString(floor(getAverageFps())) + " fps) Qtime");
}

void QuickTimePlayer::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableAlphaBlending();

	if( mFrameTexture ) {
		Rectf centeredRect = Rectf( mFrameTexture.getBounds() ).getCenteredFit( getWindowBounds(), true );
		gl::draw( mFrameTexture, centeredRect  );
		
		if (bInitialized)
		{
			spoutsender.SendTexture(mFrameTexture.getId(), mFrameTexture.getTarget(), g_Width, g_Height, false);
		}
	}

	if( mInfoTexture ) {
		glDisable( GL_TEXTURE_RECTANGLE_ARB );
		gl::draw( mInfoTexture, Vec2f( 20, getWindowHeight() - 20 - mInfoTexture.getHeight() ) );
	}
}

CINDER_APP_BASIC( QuickTimePlayer, RendererGl(0) );