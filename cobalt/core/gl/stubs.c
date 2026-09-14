#include "cobalt.h"
#include "attributes.h"


#define GLAPI EXPORT
#define APIENTRY
//TODO check this later ?
#define STUB errorShim(GL_INVALID_VALUE);

NonAliasExportDecl(void,glClampColorARB,(GLenum target, GLenum clamp)){STUB}
