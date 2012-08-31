/**
 * @file
 * 
 * Abstract base class to derive WebAppBase functionality from
 *
 * @author Hewlett-Packard Development Company, L.P.
 * @author tyrok1
 *
 * @section LICENSE
 *
 *      Copyright (c) 2008-2012 Hewlett-Packard Development Company, L.P.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */




#ifndef WEBPAGECLIENT_H
#define WEBPAGECLIENT_H

#include "Common.h"

#include <stdint.h>

#include <palmimedefines.h>
#include <palmwebviewclient.h>
#include <palmwebtypes.h>

class WebPage;
class ApplicationDescription;

namespace Palm {
	class WebGLES2Context;
}

/**
 * Abstract base class to derive WebAppBase functionality from
 * 
 * @todo Figure out why this is not set up to use smart pointers (sptr<>) for the page it attaches to.  If it did, it would be much safer for the caller since they would be far less likely to be holding an invalid pointer after our instance is destroyed.
 */
class WebPageClient
{
public:
	/**
	 * Initializes this class instance
	 * 
	 * Doesn't do anything - see derived classes'
	 * documentation for specifics.
	 */
	WebPageClient() {}
	
	/**
	 * Cleans up this class instance
	 * 
	 * Doesn't do anything - see derived classes'
	 * documentation for specifics.
	 */
	virtual ~WebPageClient() {}
	
	/**
	 * Attaches to a WebPage to allow us to control attributes of it
	 * 
	 * This method is designed to set up the content to
	 * display in this WebPageClient instance.  See
	 * specific derived classes' docs for more info.
	 * 
	 * @param	page			The page to display within our container.
	 */
	virtual void attach(WebPage* page) = 0;
	
	/**
	 * Detaches this instance from the WebPage it was previously managing
	 * 
	 * @return				The previously-managed WebPage instance containing this app's content.
	 */
	virtual WebPage* detach() = 0;
	
	/**
	 * Purpose is unclear
	 * 
	 * Implemented in WebAppBase as a protected method
	 * that returns a constant.
	 * 
	 * @todo Find a reason for this to exist and remove it if unnecessary.  If necessary, document this further.
	 * 
	 * @return				Unknown.
	 */
	virtual int  getKey() const = 0;

	/**
	 * Bring this app to the foreground
	 */
	virtual void focus() = 0;
	
	/**
	 * Request that our app not be the foreground app anymore
	 */
	virtual void unfocus() = 0;
	
	/**
	 * Asks WebAppManager to close our app for us
	 * 
	 * @see WebAppManager
	 */
	virtual void close() = 0;
	
	/**
	 * Returns the current window's size
	 * 
	 * The current window size appears to be the
	 * current outer dimensions of a card, app,
	 * or anything else being displayed in a
	 * WebPageClient.
	 * 
	 * @param	width			Width of current window, in pixels.
	 * @param	height			Height of current window, in pixels.
	 */
	virtual void windowSize(int& width, int& height) = 0;
	
	/**
	 * Unknown at this point
	 * 
	 * @todo Document this once an actual use for it is found.
	 */
	virtual void resizedContents(int contentsWidth, int contentsHeight) = 0;
	
	/**
	 * Unknown at this point
	 * 
	 * @todo Document this once an actual use for it is found.
	 */
	virtual void zoomedContents(double scaleFactor, int contentsWidth, int contentsHeight,
								int newScrollOffsetX, int newScrollOffsetY) = 0;
	/**
	 * Invalidate a portion of the display and ask for it to be repainted
	 * 
	 * Should be called any time this window is
	 * overlapped or otherwise has a portion of it
	 * covered where it needs to be redrawn.
	 * 
	 * This method clips the rectangle to the
	 * window and adds it to the redraw list.
	 * Then, it sets a timer to redraw that portion.
	 * 
	 * @note Coordinates are given in window-space pixels.
	 * 
	 * @param	x			Left coordinate of the rectangle to repaint.
	 * @param	y			Top coordinate of the rectangle to repaint.
	 * @param	width			Width of rectangle to repaint.
	 * @param	height			Height of rectangle to repaint.
	 */
	virtual void invalContents(int x, int y, int width, int height) = 0;
	
	/**
	 * Unknown at this time
	 * 
	 * Doesn't appear to actually be used anywhere.
	 * 
	 * @todo See if we can remove this.
	 * 
	 * @param	newContentsX		Unknown.
	 * @param	newContentsY		Unknown.
	 */
	virtual void scrolledContents(int newContentsX, int newContentsY) = 0;
	
	/**
	 * Should be called any time that the URI of the displayed page has changed
	 * 
	 * Updates us to let us know that our page now
	 * has a different URI.  Someone should call
	 * this method at least once after the contents
	 * of this class are displaying a different URI.
	 * 
	 * @param	url			New URI we're looking at.
	 */
	virtual void uriChanged(const char* url) = 0;
	
	/**
	 * Unknown at this time
	 * 
	 * Doesn't appear to actually be used anywhere.
	 * 
	 * @todo See if we can remove this.
	 * 
	 * @param	title			Presumably the new title of the contents of the page we're displaying.
	 */
	virtual void titleChanged(const char* title) = 0;
	
	/**
	 * Unknown at this time
	 * 
	 * Doesn't appear to actually be used anywhere.
	 * 
	 * @todo See if we can remove this.
	 * 
	 * @param	message			Presumably the text for the status bar of the page we're displaying.
	 */
	virtual void statusMessage(const char* message) = 0;
	
	/**
	 * Presumably dispatches information about a page failing to load to something listening for it
	 * 
	 * This is pretty much implemented everywhere as
	 * empty and doesn't seem to be called from
	 * anywhere.  It appears to either be
	 * functionality that has since been removed or
	 * that was never fully implemented in the first
	 * place.
	 * 
	 * @todo See if this can be removed entirely.
	 * 
	 * @param	domain			Unknown.
	 * @param	errorCode		Unknown.
	 * @param	failingURL		Unknown.
	 * @param	localizedDescription	Unknown.
	 */
	virtual void dispatchFailedLoad(const char* domain, int errorCode,
			const char* failingURL, const char* localizedDescription) = 0;
	
	/**
	 * Finish up any tasks to be completed when a page finishes loading
	 * 
	 * This method should be called any time that
	 * this page is finished loading, be it by it
	 * actually successfully loading a page, by
	 * stop being tapped, or by closing the
	 * window.
	 */
	virtual void loadFinished() = 0;

	/**
	 * Called by the WebPage we're displaying so it can let us know when the text editor system's state changes
	 * 
	 * @todo Document this more fully once PalmIME::EditorState is documented.
	 * 
	 * @param	focused			Whether or not an editable field is currently focused on.
	 * @param	state			The state of the editor.
	 */
	virtual void editorFocusChanged(bool focused, const PalmIME::EditorState& state) = 0;
	
	/**
	 * Enables/disables auto-capitalization in fields in this app
	 * 
	 * @param	enabled			true to enable auto-capitalization, false to disable it.
	 */
	virtual void autoCapEnabled(bool enabled) = 0;
	
	/**
	 * Enables/disables whether touch events should be passed to the scripts running on this page
	 * 
	 * @note Can be overridden in a Luna preference - see CardWindow::onEnableTouchEvents().
	 * 
	 * @see CardWindow::onEnableTouchEvents()
	 * 
	 * @param	needTouchEvents		true to enable touch events, false to disable them.
	 */
	virtual void needTouchEvents(bool needTouchEvents) = 0;
	
	/**
	 * Returns a pointer to an ApplicationDescription instance describing this app - basically a representation of its appinfo.json file
	 * 
	 * @see ApplicationDescription
	 * 
	 * @return				Pointer to ApplicationDescription instance containing information about this app.
	 */
	virtual ApplicationDescription* getAppDescription() = 0;
	
	/**
	 * Gets an OpenGL ES context pointer for drawing to the display for this app
	 * 
	 * @todo Document this a bit further once Palm::WebGLES2Context and classes deriving from this one are documented further.
	 * 
	 * @return				OpenGL ES context pointer.
	 */
	virtual Palm::WebGLES2Context* getGLES2Context() = 0;
	
	/**
	 * Disables drawing of the contents of this page, even if requested
	 * 
	 * Can be overridden in some cases.
	 * See CardWebApp::forcePaint().
	 * 
	 * @see CardWebApp::forcePaint()
	 * @see WebPageClient::resumeAppRendering()
	 */
	virtual void suspendAppRendering() = 0;
	
	/**
	 * Re-enables drawing of the contents of this page
	 * 
	 * @see WebPageClient::suspendAppRendering()
	 */
	virtual void resumeAppRendering() = 0;
	
	/**
	 * Returns the screen width and height as reported by WebAppManager
	 * 
	 * Wraps
	 * {@link WebAppManager::currentUiWidth() WebAppManager::currentUiWidth()}
	 * and
	 * {@link WebAppManager::currentUiHeight() WebAppManager::currentUiHeight()}.
	 * 
	 * @see WebAppManager::currentUiWidth()
	 * @see WebAppManager::currentUiHeight()
	 * 
	 * @param	width			Width of screen, in pixels, as reported by WebAppManager.
	 * @param	height			Height of screen, in pixels, as reported by WebAppManager.
	 */
	virtual void screenSize(int& width, int& height) = 0;
	
	/**
	 * Function initializes(opens)/De-initializes the appropriate sensor
	 *
	 * @param[in] aSensorType   - Sensor Type
	 * @param[in] aNeedEvents   - flag indicating whether to start or stop the sensor
	 *
	 * @return true if successful, false otherwise
	 */
	virtual bool enableSensor(Palm::SensorType aSensorType, bool aNeedEvents) = 0;
	
	/**
	 * Function sets the Accelerometer rate to highest level.
	 * Currently the there is no means of setting fastAccelerometer to OFF.
	 * So, the enable argument is not utilized at this point of time.
	 *
	 * @param[in] enable   - Whether to enable/disable
	 */
	virtual void fastAccelerometerOn(bool enable) = 0;
};	

#endif /* WEBPAGECLIENT_H */
