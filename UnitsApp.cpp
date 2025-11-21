#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/TileRender.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"

#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "Resources.h"

#include "Net.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class UnitsApp : public AppNative {
public:
    void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
    void keyDown( KeyEvent event );
	void update();
	void draw();
    void saveFrame();
    void renderTiles();
};

int n_units(100);
int border(0);
double target(0.0);
double value_range(20.0);

bool colormode(true);
bool color(true);

bool render(false);

int framecount;

float x,y;

Net mynet(n_units, target, value_range);

void UnitsApp::prepareSettings( Settings *settings ) {
    settings->setFrameRate(60.0f);
    settings->setWindowSize(1000,1000);
}

void UnitsApp::setup()
{
    x = getWindowWidth()/n_units;
    y = getWindowHeight()/n_units;
    framecount=0;
    mynet.rand_unit_values();
    mynet.rand_unit_targets();
    gl::clear( Color( 0, 0, 0 ) );
    
}

void UnitsApp::mouseDown( MouseEvent event )
{
}

void UnitsApp::keyDown( KeyEvent event)
{
    if( event.getChar() == ' ' ) mynet.update();
    if( event.getChar() == 'b' ) mynet.rand_unit_values();
    if( event.getChar() == 'v' ) mynet.rand_unit_targets();
    if( event.getChar() == 'c' ) mynet.reset_targets();
    if( event.getChar() == 'g' ) mynet.same_values();
    if( event.getChar() == 'h' ) colormode = !colormode;
    if( event.getChar() == 'r' ) mynet.reset();
    if( event.getChar() == 'j' ) color = !color;
}

void UnitsApp::update()
{
    mynet.update();
    //mynet.print();
    
    if(render) renderTiles();

}

void UnitsApp::draw()
{
    float r(0.0),g(0.0),b(0.0);
    
    for(auto unit: mynet.m_units)
    {
        float cv(unit->m_value);
        
        cv = cinder::lmap<float>(cv, -value_range, value_range, 0.0, 1.0);
        
        //double unit_delta(unit->m_target - unit->m_value);
        double unit_delta(unit->delta);
        
        if(!colormode)
        {
            r = 0.0;
            g = 0.0;
            b = 0.0;
        }
        
        //unit_delta = 100.0 * unit_delta/value_range;
        
        /*
         b = unit_delta;
         r = -unit_delta;
         */
        
        if(unit_delta > 0) b = unit_delta;
        if(unit_delta < 0) r = -unit_delta;
        
        
        g = (g+fabs(unit_delta) + cv)/3;
        r = (r+cv)/2;
        b = (b+cv)/2;
        
        if(color) gl::color(r, g, b, 1.0);
        else gl::color(g,g,g, 1.0);
        
        
        Vec2f p_min(x * unit->x + border, y * unit->y + border);
        Vec2f p_max(x * unit->x + x - border, y * unit->y + y - border);
        
        gl::drawSolidRect(RectT<float>(p_min, p_max));
    }
}

void UnitsApp::saveFrame()
{
    
    try {
        
        fs::path pngPath = "/Users/Clemens/Pictures/units/units_"+toString(framecount++)+".png";
        console() << pngPath << std::endl;
        if( ! pngPath.empty() ) {
            writeImage( pngPath, cinder::app::copyWindowSurface(), ImageTarget::Options().colorModel( ImageIo::CM_RGB ).quality( 0.5f ) );
        }
    }
    catch( ... ) {
        
    }
}

void UnitsApp::renderTiles()
{
	//the size here doesn't matter, but it will get distorted if it's not the same ratio as your window
	gl::TileRender tr( getWindowWidth(), getWindowHeight() );
	//use the default cinder view to render from
	tr.setMatricesWindow(getWindowWidth(), getWindowHeight());
    
	while( tr.nextTile() ) {
		draw();
	}
	writeImage( "/Users/Clemens/Pictures/units2/units_"+toString(framecount++)+".png", tr.getSurface() );
    
	//reset window matrix back to normal (if you aren't changing it with a camera)
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight(), true );
}

;


CINDER_APP_NATIVE( UnitsApp, RendererGl )
