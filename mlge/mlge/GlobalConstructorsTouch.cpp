/**
* 
* All this file does is include all the engine's headers that define objects, so the constructors of static global variables are
* called.
* 
* An example on why this is needed is that if a new resource type but not referenced from the game's code, then the static
* variables for that resource type are not constructed and thus the resource type is not registered, causing resource loading to
* fail
*
*/

#include "mlge/Resource/TTFFont.h"
#include "mlge/Resource/Texture.h"
#include "mlge/Resource/SpriteSheet.h"
#include "mlge/Resource/Resource.h"
#include "mlge/Resource/Image.h"
#include "mlge/Resource/Flipbook.h"
#include "mlge/Render/RenderQueue.h"
#include "mlge/TextureComponent.h"
#include "mlge/Text.h"
#include "mlge/Level.h"
#include "mlge/FlipbookComponent.h"
#include "mlge/ActorComponent.h"
#include "mlge/Actor.h"

namespace mlge
{

void globalConstructorsTouch()
{
}

}

