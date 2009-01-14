#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

#include "Grit.h"
#include "lua_wrappers_scnmgr.h"
#include "lua_wrappers_mobj.h"
#include "lua_wrappers_primitives.h"
#include "lua_wrappers_material.h"
#include "lua_wrappers_tex.h"

#include "lua_userdata_dependency_tracker_funcs.h"

// SCENE NODE ============================================================== {{{

void push_node (lua_State *L, Ogre::SceneNode *n)
{
        if (!n) {
                lua_pushnil(L);
                return;
        }
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (n);
        Ogre::SceneManager *scnmgr = n->getCreator();
        luaL_getmetatable(L, NODE_TAG);
        lua_setmetatable(L, -2);
        APP_ASSERT(scnmgr!=NULL);
        APP_ASSERT(n!=NULL);
        APP_ASSERT(ud!=NULL);
        grit->getUserDataTables().scnmgrs[scnmgr].sceneNodes[n].push_back(ud);
}

static int node_child_node (lua_State *L)
{
TRY_START
        std::string name;
        if (lua_gettop(L)!=1)
                check_args(L,2);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        if (lua_gettop(L)==1) {
                push_node(L,self.createChildSceneNode());
        } else {
                std::string name = luaL_checkstring(L,2);
                push_node(L,self.createChildSceneNode(name));
        }
        return 1;
TRY_END
}

static int node_translate (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) {
                GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
                Ogre::Real px = luaL_checknumber(L,2);
                Ogre::Real py = luaL_checknumber(L,3);
                Ogre::Real pz = luaL_checknumber(L,4);
                self.translate(Ogre::Vector3(px,py,pz));
                return 0;
        }
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        GET_UD_MACRO(Ogre::Vector3,v,2,VECTOR3_TAG);
        self.translate(v);
        return 0;
TRY_END
}

static int node_set_position_orientation (lua_State *L)
{
TRY_START
        check_args(L,8);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        Ogre::Real px = luaL_checknumber(L,2);
        Ogre::Real py = luaL_checknumber(L,3);
        Ogre::Real pz = luaL_checknumber(L,4);
        Ogre::Real rw = luaL_checknumber(L,5);
        Ogre::Real rx = luaL_checknumber(L,6);
        Ogre::Real ry = luaL_checknumber(L,7);
        Ogre::Real rz = luaL_checknumber(L,8);
        self.setPosition(px,py,pz);
        self.setOrientation(rw,rx,ry,rz);
        return 0;
TRY_END
}

static int node_detach_all_objects (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        self.detachAllObjects();
        return 0;
TRY_END
}

static int node_add_child (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        GET_UD_MACRO(Ogre::SceneNode,child,2,NODE_TAG);
        self.addChild(&child);
        return 0;
TRY_END
}

static int node_remove_child (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        GET_UD_MACRO(Ogre::SceneNode,child,2,NODE_TAG);
        self.removeChild(&child);
        return 0;
TRY_END
}

static int node_remove_all_children (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        self.removeAllChildren();
        return 0;
TRY_END
}

static int node_detach_object (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        Ogre::MovableObject *mobj = check_mobj(L,2);
        self.detachObject(mobj);
        return 0;
TRY_END
}

static int node_attach_object (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        Ogre::MovableObject *mobj = check_mobj(L,2);
        Ogre::SceneNode::ObjectIterator i =
                self.getAttachedObjectIterator();
        while (i.hasMoreElements()) {
                Ogre::MovableObject *o = i.getNext();
                if (o->getName()==mobj->getName()) {
                        my_lua_error(L,"Object of this name already attached "
                                       "to SceneNode: "+o->getName());
                }
        }
        self.attachObject(mobj);
        return 0;
TRY_END
}

static int node_destroy (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        Ogre::SceneManager *scnmgr = self.getCreator();
        scnmgr->destroySceneNode(self.getName());
        map_nullify_remove(
                          grit->getUserDataTables().scnmgrs[scnmgr].sceneNodes,
                           &self);
        return 0;
TRY_END
}


TOSTRING_GETNAME_MACRO(node,Ogre::SceneNode,.getName(),NODE_TAG)


static int node_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::SceneNode,self,1,NODE_TAG,0);
        if (self==NULL) return 0;
        Ogre::SceneManager *scnmgr = self->getCreator();
        vec_nullify_remove(
                grit->getUserDataTables().scnmgrs[scnmgr].sceneNodes[self],
                &self);
        return 0;
TRY_END
}

static int node_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="createChild") {
                push_cfunction(L,node_child_node);
        } else if (key=="position") {
                push(L,new Ogre::Vector3(self.getPosition()),VECTOR3_TAG);
        } else if (key=="orientation") {
                push(L,new Ogre::Quaternion(self.getOrientation()),QUAT_TAG);
        } else if (key=="scale") {
                push(L,new Ogre::Vector3(self.getScale()),VECTOR3_TAG);
        } else if (key=="derivedPosition") {
                push(L,new Ogre::Vector3(self._getDerivedPosition()),
                     VECTOR3_TAG);
        } else if (key=="derivedScale") {
                push(L,new Ogre::Vector3(self._getDerivedScale()),
                     VECTOR3_TAG);
        } else if (key=="derivedOrientation") {
                push(L,new Ogre::Quaternion(self._getDerivedOrientation()),
                       QUAT_TAG);
        } else if (key=="inheritOrientation") {
                lua_pushboolean(L,self.getInheritOrientation());
        } else if (key=="inheritScale") {
                lua_pushboolean(L,self.getInheritScale());
        } else if (key=="translate") {
                push_cfunction(L,node_translate);
        } else if (key=="setPositionOrientation") {
                push_cfunction(L,node_set_position_orientation);
        } else if (key=="attachObject") {
                push_cfunction(L,node_attach_object);
        } else if (key=="detachObject") {
                push_cfunction(L,node_detach_object);
        } else if (key=="detachAllObjects") {
                push_cfunction(L,node_detach_all_objects);
        } else if (key=="addChild") {
                push_cfunction(L,node_add_child);
        } else if (key=="removeChild") {
                push_cfunction(L,node_remove_child);
        } else if (key=="removeAllChildren") {
                push_cfunction(L,node_remove_all_children);
        } else if (key=="objects") {
                lua_newtable(L);
                int counter = 0;
                Ogre::SceneNode::ObjectIterator i =
                        self.getAttachedObjectIterator();
                while (i.hasMoreElements()) {
                        Ogre::MovableObject *o = i.getNext();
                        if (push_mobj(L,o)) {
                                lua_rawseti(L,-2,counter+LUA_ARRAY_BASE);
                                counter++;
                        }
                }
        } else if (key=="children") {
                lua_newtable(L);
                int counter = 0;
                Ogre::SceneNode::ChildNodeIterator j =
                        self.getChildIterator();
                while (j.hasMoreElements()) {
                        Ogre::Node *o = j.getNext();
                        if (dynamic_cast<Ogre::SceneNode*>(o)) {
                                push_node(L, static_cast<Ogre::SceneNode*>(o));
                                lua_rawseti(L,-2,counter+LUA_ARRAY_BASE);
                                counter++;
                        }
                }
        } else if (key == "parentSceneNode") {
                push_node(L,self.getParentSceneNode());
        } else if (key=="destroy") {
                push_cfunction(L,node_destroy);
        } else {
                my_lua_error(L,"Not a valid SceneNode member: "+key);
        }
        return 1;
TRY_END
}

static int node_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::SceneNode,self,1,NODE_TAG);
        std::string key  = luaL_checkstring(L,2);
        if (key=="position") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self.setPosition(v);
        } else if (key=="orientation") {
                GET_UD_MACRO(Ogre::Quaternion,q,3,QUAT_TAG);
                self.setOrientation(q);
        } else if (key=="scale") {
                GET_UD_MACRO(Ogre::Vector3,v,3,VECTOR3_TAG);
                self.setScale(v);
        } else if (key=="inheritOrientation") {
                bool v = 0!=lua_toboolean(L,3);
                self.setInheritOrientation(v);
        } else if (key=="inheritScale") {
                bool v = 0!=lua_toboolean(L,3);
                self.setInheritScale(v);
        } else {
                my_lua_error(L,"Not a valid SceneNode member: "+key);
        }
        return 0;
TRY_END
}

EQ_PTR_MACRO(Ogre::SceneNode,node,NODE_TAG)

MT_MACRO_NEWINDEX(node);

//}}}



// SCENE MANAGER =========================================================== {{{

void push_scnmgr (lua_State *L, Ogre::SceneManager *scnmgr)
{
        if (!scnmgr) {
                lua_pushnil(L);
                return;
        }
        void **ud = static_cast<void**>(lua_newuserdata(L, sizeof(*ud)));
        ud[0] = static_cast<void*> (scnmgr);
        luaL_getmetatable(L, SCNMGR_TAG);
        lua_setmetatable(L, -2);
        grit->getUserDataTables().scnmgrs[scnmgr].userData.push_back(ud);
}

static int scnmgr_set_skybox (lua_State *L)
{
TRY_START

        int num_args = lua_gettop(L);
        if (num_args<3 || num_args>6)
                my_lua_error(L,"Incorrect number of arguments");

        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        bool enable = 0!=lua_toboolean(L,2);
        std::string mat_name(luaL_checkstring(L,3));
        Ogre::Real dist = 100;
        bool draw_first = true;
        Ogre::Quaternion orientation=Ogre::Quaternion::IDENTITY;

        switch (num_args) {
                case 6: {
                        GET_UD_MACRO(Ogre::Quaternion,q,6,QUAT_TAG);
                        orientation = q;
                }
                case 5:
                draw_first = 0!=lua_toboolean(L,5);
                case 4:
                dist = luaL_checknumber(L,4);
                case 3:
                break;
        }

        self.setSkyBox(enable,mat_name,dist,draw_first,orientation,"GRIT");
        return 0;
TRY_END
}


static int scnmgr_create_light (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        Ogre::Light *light = self.createLight(name);
        push_light(L,light);
        return 1;
TRY_END
}

static int scnmgr_get_light (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        if (!self.hasLight(name)) {
                lua_pushnil(L);
                return 1;
        }
        Ogre::Light *light = self.getLight(name);
        push_light(L,light);
        return 1;
TRY_END
}


static int scnmgr_create_cam (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        Ogre::Camera *cam = self.createCamera(name);
        push_cam(L,cam);
        return 1;
TRY_END
}

static int scnmgr_get_cam (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        if (!self.hasCamera(name)) {
                lua_pushnil(L);
                return 1;
        }
        Ogre::Camera *cam = self.getCamera(name);
        push_cam(L,cam);
        return 1;
TRY_END
}


static int scnmgr_get_manobj (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        if (!self.hasManualObject(name)) {
                lua_pushnil(L);
                return 1;
        }
        Ogre::ManualObject *o = self.getManualObject(name);
        push_manobj(L,o);
        return 1;
TRY_END
}


static int scnmgr_create_node (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        Ogre::SceneNode *node = self.createSceneNode();
        push_node(L,node);
        return 1;
TRY_END
}

static int scnmgr_get_node (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        if (!self.hasSceneNode(name)) {
                lua_pushnil(L);
                return 1;
        }
        Ogre::SceneNode *node = self.getSceneNode(name);
        push_node(L,node);
        return 1;
TRY_END
}


static int scnmgr_create_instgeo (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        Ogre::InstancedGeometry *g = self.createInstancedGeometry(name);
        push_instgeo(L,&self,g);
        return 1;
TRY_END
}

static int scnmgr_get_instgeo (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        //if (!self.hasInstancedGeometry(name)) {
        //        lua_pushnil(L);
        //        return 1;
        //}
        Ogre::InstancedGeometry *instgeo = self.getInstancedGeometry(name);
        push_instgeo(L,&self,instgeo);
        return 1;
TRY_END
}

static int scnmgr_create_statgeo (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        Ogre::StaticGeometry *g = self.createStaticGeometry(name);
        push_statgeo(L,&self,g);
        return 1;
TRY_END
}

static int scnmgr_get_statgeo (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        if (!self.hasStaticGeometry(name)) {
                lua_pushnil(L);
                return 1;
        }
        Ogre::StaticGeometry *statgeo = self.getStaticGeometry(name);
        push_statgeo(L,&self,statgeo);
        return 1;
TRY_END
}

static int scnmgr_get_entity (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *ent_name = luaL_checkstring(L,2);
        if (!self.hasEntity(ent_name)) {
                lua_pushnil(L);
                return 1;
        }
        Ogre::Entity *ent = self.getEntity(ent_name);
        push_entity(L,ent);
        return 1;
TRY_END
}

static int scnmgr_create_entity (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *ent_name = luaL_checkstring(L,2);
        const char *mesh_name = luaL_checkstring(L,3);
        push_entity(L,self.createEntity(ent_name,mesh_name));
        return 1;
TRY_END
}

static int scnmgr_has_mobj (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        const char *type = luaL_checkstring(L,3);
        lua_pushboolean(L,self.hasMovableObject(name,type));
        return 1;
TRY_END
}

static int scnmgr_get_mobj (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        const char *type = luaL_checkstring(L,3);
        if (!self.hasMovableObject(name,type)) {
                lua_pushnil(L);
                return 1;
        }
        Ogre::MovableObject *mobj = self.getMovableObject(name,type);
        if (!push_mobj(L,mobj))
                lua_pushnil(L);
        return 1;
TRY_END
}

static int scnmgr_create_mobj (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const char *name = luaL_checkstring(L,2);
        const char *type = luaL_checkstring(L,3);
        if (!lua_istable(L,4))
                my_lua_error(L,"Last parameter should be a table");

        Ogre::NameValuePairList params;
        // scan through table adding k/v pairs to params
        for (lua_pushnil(L) ; lua_next(L,4)!=0 ; lua_pop(L,1)) {
                Ogre::String value = luaL_checkstring(L,-1);
                Ogre::String key = luaL_checkstring(L,-2);
                params[key] = value;
        }

        Ogre::MovableObject *mobj = self.createMovableObject(name,type,&params);
        if (!push_mobj(L,mobj))
                lua_pushnil(L);
        return 1;
TRY_END
}

static int scnmgr_get_fog (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        lua_pushstring(L,fog_mode_to_string(L,self.getFogMode()));
        const Ogre::ColourValue &cv = self.getFogColour();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        lua_pushnumber(L,self.getFogDensity());
        lua_pushnumber(L,self.getFogStart());
        lua_pushnumber(L,self.getFogEnd());
        return 7;
TRY_END
}

static int scnmgr_set_fog (lua_State *L)
{
TRY_START
        check_args(L,8);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        Ogre::FogMode fm = fog_mode_from_string(L,luaL_checkstring(L,2));
        Ogre::Real r = luaL_checknumber(L,3);
        Ogre::Real g = luaL_checknumber(L,4);
        Ogre::Real b = luaL_checknumber(L,5);
        Ogre::Real density = luaL_checknumber(L,6);
        Ogre::Real start = luaL_checknumber(L,7);
        Ogre::Real end = luaL_checknumber(L,8);
        self.setFog(fm,Ogre::ColourValue(r,g,b),density,start,end);
        return 0;
TRY_END
}

static int scnmgr_get_ambient (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const Ogre::ColourValue &cv = self.getAmbientLight();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        return 3;
TRY_END
}

static int scnmgr_set_ambient (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        Ogre::Real r = luaL_checknumber(L,2);
        Ogre::Real g = luaL_checknumber(L,3);
        Ogre::Real b = luaL_checknumber(L,4);
        self.setAmbientLight(Ogre::ColourValue(r,g,b));
        return 0;
TRY_END
}

static int scnmgr_get_shadow_colour (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        const Ogre::ColourValue &cv = self.getShadowColour();
        lua_pushnumber(L,cv.r);
        lua_pushnumber(L,cv.g);
        lua_pushnumber(L,cv.b);
        return 3;
TRY_END
}

static int scnmgr_set_shadow_colour (lua_State *L)
{
TRY_START
        check_args(L,4);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        Ogre::Real r = luaL_checknumber(L,2);
        Ogre::Real g = luaL_checknumber(L,3);
        Ogre::Real b = luaL_checknumber(L,4);
        self.setShadowColour(Ogre::ColourValue(r,g,b));
        return 0;
TRY_END
}

static int scnmgr_get_shadow_texture (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        size_t index = check_t<size_t>(L,2);
        push(L, new Ogre::TexturePtr(self.getShadowTexture(index)),TEX_TAG);
        return 1;
TRY_END
}

static const char *
shadow_type_to_string (lua_State *L, Ogre::ShadowTechnique st)
{
        switch (st) {
        case Ogre::SHADOWTYPE_NONE: return "NONE";
        case Ogre::SHADOWTYPE_STENCIL_MODULATIVE: return "STENCIL_MODULATIVE";
        case Ogre::SHADOWTYPE_STENCIL_ADDITIVE: return "STENCIL_ADDITIVE";
        case Ogre::SHADOWTYPE_TEXTURE_MODULATIVE: return "TEXTURE_MODULATIVE";
        case Ogre::SHADOWTYPE_TEXTURE_ADDITIVE: return "TEXTURE_ADDITIVE";
        case Ogre::SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED:
                return "TEXTURE_MODULATIVE_INTEGRATED";
        case Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED:
                return "TEXTURE_ADDITIVE_INTEGRATED";
        default:
                my_lua_error(L,"Unrecognised shadow type.");
                return "never happens";
        }
}
static Ogre::ShadowTechnique
shadow_type_from_string (lua_State *L, const std::string& t)
{
        if (t=="NONE") {
                return Ogre::SHADOWTYPE_NONE;
        } else if (t=="STENCIL_MODULATIVE") {
                return Ogre::SHADOWTYPE_STENCIL_MODULATIVE;
        } else if (t=="STENCIL_ADDITIVE") {
                return Ogre::SHADOWTYPE_STENCIL_ADDITIVE;
        } else if (t=="TEXTURE_MODULATIVE") {
                return Ogre::SHADOWTYPE_TEXTURE_MODULATIVE;
        } else if (t=="TEXTURE_ADDITIVE") {
                return Ogre::SHADOWTYPE_TEXTURE_ADDITIVE;
        } else if (t=="TEXTURE_MODULATIVE_INTEGRATED") {
                return Ogre::SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED;
        } else if (t=="TEXTURE_ADDITIVE_INTEGRATED") {
                return Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED;
        } else {
                my_lua_error(L,"Unrecognised shadow type: "+t);
                return Ogre::SHADOWTYPE_NONE;//never happens
        }
}


static int scnmgr_update_scene_graph (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        self.getRootSceneNode()->_update(true,false);
        return 0;
TRY_END
}


static int scnmgr_destroy(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        grit->getOgre()->destroySceneManager(&self);
        struct scnmgr_maps& maps = grit->getUserDataTables().scnmgrs[&self];
        vec_nullify_remove_all(maps.userData);
        map_nullify_remove_all(maps.sceneNodes);
        map_nullify_remove_all(maps.cameras);
        map_nullify_remove_all(maps.entities);
        map_nullify_remove_all(maps.lights);
        map_nullify_remove_all(maps.manobjs);
        map_nullify_remove_all(maps.statgeoms);
        map_nullify_remove_all(maps.instgeoms);
        map_remove_only(grit->getUserDataTables().scnmgrs,&self);
        return 0;
TRY_END
}


TOSTRING_ADDR_MACRO(scnmgr,Ogre::SceneManager,SCNMGR_TAG)


static int scnmgr_gc(lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(Ogre::SceneManager,self,1,SCNMGR_TAG,0);
        if (self==NULL) return 0;
        vec_nullify_remove(grit->getUserDataTables().scnmgrs[self].userData,
                           &self);
        return 0;
TRY_END
}

static int scnmgr_index(lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        //FPSceneManager *fp = dynamic_cast<FPSceneManager*>(&self);
        std::string key  = luaL_checkstring(L,2);

        if (key=="setSkyBox") {
                push_cfunction(L,scnmgr_set_skybox);

        } else if (key=="shadowTechnique") {
                lua_pushstring(L,
                        shadow_type_to_string(L,self.getShadowTechnique()));
        } else if (key=="shadowDirectionalLightExtrusionDistance") {
                lua_pushnumber(L,
                        self.getShadowDirectionalLightExtrusionDistance());
        } else if (key=="shadowFarDistance") {
                lua_pushnumber(L,self.getShadowFarDistance());
        } else if (key=="shadowIndexBufferSize") {
                lua_pushnumber(L,self.getShadowIndexBufferSize());
        } else if (key=="shadowTextureCount") {
                lua_pushnumber(L,self.getShadowTextureCount());
        } else if (key=="getShadowTexture") {
                push_cfunction(L,scnmgr_get_shadow_texture);
        } else if (key=="shadowDirLightTextureOffset") {
                lua_pushnumber(L,self.getShadowDirLightTextureOffset());
/* NOT IMPLEMENTED?!
        } else if (key=="shadowTextureFadeStart") {
                lua_pushnumber(L,self.getShadowTextureFadeStart());
        } else if (key=="shadowTextureFadeEnd") {
                lua_pushnumber(L,self.getShadowTextureFadeEnd());
*/
        } else if (key=="shadowTextureSelfShadow") {
                lua_pushboolean(L,self.getShadowTextureSelfShadow());
        } else if (key=="shadowCasterRenderBackFaces") {
                lua_pushboolean(L,self.getShadowCasterRenderBackFaces());
/*
        } else if (key=="shadowUseInfiniteFarPlane") {
                lua_pushboolean(L,self.getShadowUseInfiniteFarPlane());
*/

        } else if (key=="setShadowColour") {
                push_cfunction(L,scnmgr_set_shadow_colour);
        } else if (key=="getShadowColour") {
                push_cfunction(L,scnmgr_get_shadow_colour);

        } else if (key=="setAmbientLight") {
                push_cfunction(L,scnmgr_set_ambient);
        } else if (key=="getAmbientLight") {
                push_cfunction(L,scnmgr_get_ambient);
        } else if (key=="setFog") {
                push_cfunction(L,scnmgr_set_fog);
        } else if (key=="getFog") {
                push_cfunction(L,scnmgr_get_fog);

        } else if (key=="createLight") {
                push_cfunction(L,scnmgr_create_light);
        } else if (key=="getLight") {
                push_cfunction(L,scnmgr_get_light);

        } else if (key=="createCamera") {
                push_cfunction(L,scnmgr_create_cam);
        } else if (key=="getCamera") {
                push_cfunction(L,scnmgr_get_cam);

        } else if (key=="createNode") {
                push_cfunction(L,scnmgr_create_node);
        } else if (key=="getNode") {
                push_cfunction(L,scnmgr_get_node);

        } else if (key=="getManualObject") {
                push_cfunction(L,scnmgr_get_manobj);

        } else if (key=="entities") {
                lua_newtable(L);
                unsigned int c = 0;
                {
                        Ogre::SceneManager::MovableObjectIterator i =
                                self.getMovableObjectIterator("Entity");
                        while (i.hasMoreElements()) {
                                Ogre::Entity *ent = static_cast<Ogre::Entity*>
                                                                (i.getNext());
                                push_entity(L,ent);
                                lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                                c++;
                        }
                }
                {
                        Ogre::SceneManager::MovableObjectIterator i =
                                self.getMovableObjectIterator("FPEntity");
                        while (i.hasMoreElements()) {
                                Ogre::Entity *ent = static_cast<Ogre::Entity*>
                                                                (i.getNext());
                                push_entity(L,ent);
                                lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                                c++;
                        }
                }

        } else if (key=="manualObjects") {
                lua_newtable(L);
                unsigned int c = 0;
                Ogre::SceneManager::MovableObjectIterator j =
                        self.getMovableObjectIterator("ManualObject");
                while (j.hasMoreElements()) {
                        Ogre::ManualObject *o =
                                static_cast<Ogre::ManualObject*>(j.getNext());
                        push_manobj(L,o);
                        lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                        c++;
                }

        } else if (key=="cameras") {
                lua_newtable(L);
                unsigned int c = 0;
                Ogre::SceneManager::MovableObjectIterator j =
                        self.getMovableObjectIterator("Camera");
                while (j.hasMoreElements()) {
                        Ogre::Camera *cam = static_cast<Ogre::Camera*>
                                                        (j.getNext());
                        push_cam(L,cam);
                        lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
                        c++;
                }

        } else if (key=="createInstancedGeometry") {
                push_cfunction(L,scnmgr_create_instgeo);
        } else if (key=="getInstancedGeometry") {
                push_cfunction(L,scnmgr_get_instgeo);

        } else if (key=="createStaticGeometry") {
                push_cfunction(L,scnmgr_create_statgeo);
        } else if (key=="getStaticGeometry") {
                push_cfunction(L,scnmgr_get_statgeo);

        } else if (key=="createEntity") {
                push_cfunction(L,scnmgr_create_entity);
        } else if (key=="getEntity") {
                push_cfunction(L,scnmgr_get_entity);

        } else if (key=="createMovableObject") {
                push_cfunction(L,scnmgr_create_mobj);
        } else if (key=="getMovableObject") {
                push_cfunction(L,scnmgr_get_mobj);
        } else if (key=="hasMovableObject") {
                push_cfunction(L,scnmgr_has_mobj);

        } else if (key=="destroy") {
                push_cfunction(L,scnmgr_destroy);

/*
        } else if (key=="fadeFarFactor") {
                if (fp==NULL) my_lua_error(L,"Not an FPSceneManager.");
                lua_pushnumber(L,fp->getFadeFarFactor());
        } else if (key=="fadeNearFactor") {
                if (fp==NULL) my_lua_error(L,"Not an FPSceneManager.");
                lua_pushnumber(L,fp->getFadeNearFactor());
        } else if (key=="loadsPerFrame") {
                if (fp==NULL) my_lua_error(L,"Not an FPSceneManager.");
                lua_pushnumber(L,fp->getLoadsPerFrame());
        } else if (key=="preloadRate") {
                if (fp==NULL) my_lua_error(L,"Not an FPSceneManager.");
                lua_pushnumber(L,fp->getPreloadRate());
        } else if (key=="visibilityRate") {
                if (fp==NULL) my_lua_error(L,"Not an FPSceneManager.");
                lua_pushnumber(L,fp->getVisibilityRate());
        } else if (key=="preloadLookAheadMax") {
                if (fp==NULL) my_lua_error(L,"Not an FPSceneManager.");
                lua_pushnumber(L,fp->getPreloadLookAheadMax());
        } else if (key=="preloadLookAheadFactor") {
                if (fp==NULL) my_lua_error(L,"Not an FPSceneManager.");
                lua_pushnumber(L,fp->getPreloadLookAheadFactor());
        } else if (key=="preloadRadiusFactor") {
                if (fp==NULL) my_lua_error(L,"Not an FPSceneManager.");
                lua_pushnumber(L,fp->getPreloadRadiusFactor());
        } else if (key=="visibilityFactor") {
                if (fp==NULL) my_lua_error(L,"Not an FPSceneManager.");
                lua_pushnumber(L,fp->getVisibilityFactor());
        } else if (key=="nearFactor") {
                if (fp==NULL) my_lua_error(L,"Not an FPSceneManager.");
                lua_pushnumber(L,fp->getNearFactor());
*/

        } else if (key=="showSceneNodes") {
                lua_pushboolean(L,self.getDisplaySceneNodes());
        } else if (key=="showBoundingBoxes") {
                lua_pushboolean(L,self.getShowBoundingBoxes());
        } else if (key=="showDebugShadows") {
                lua_pushboolean(L,self.getShowDebugShadows());

        } else if (key=="skyBoxNode") {
                push_node(L,self.getSkyBoxNode());
        } else if (key=="root") {
                push_node(L,self.getRootSceneNode());

        } else if (key=="updateSceneGraph") {
                push_cfunction(L, scnmgr_update_scene_graph);

        } else {
                my_lua_error(L,"Not a valid SceneManager member: "+key);
        }
        return 1;
TRY_END
}

static int scnmgr_newindex(lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(Ogre::SceneManager,self,1,SCNMGR_TAG);
        std::string key  = luaL_checkstring(L,2);

        if (key=="showSceneNodes") {
                self.setDisplaySceneNodes(0!=lua_toboolean(L,3));
        } else if (key=="showBoundingBoxes") {
                self.showBoundingBoxes(0!=lua_toboolean(L,3));
        } else if (key=="showDebugShadows") {
                self.setShowDebugShadows(0!=lua_toboolean(L,3));

        } else if (key=="shadowTechnique") {
                const char* st = luaL_checkstring(L,3);
                self.setShadowTechnique(shadow_type_from_string(L,st));
        } else if (key=="shadowDirectionalLightExtrusionDistance") {
                Ogre::Real v = luaL_checknumber(L,3);
                self.setShadowDirectionalLightExtrusionDistance(v);
        } else if (key=="shadowFarDistance") {
                Ogre::Real v = luaL_checknumber(L,3);
                self.setShadowFarDistance(v);
        } else if (key=="shadowIndexBufferSize") {
                size_t v = check_t<size_t>(L,3);
                self.setShadowIndexBufferSize(v);
        } else if (key=="shadowTextureCount") {
                size_t v = check_t<size_t>(L,3);
                self.setShadowTextureCount(v);
        } else if (key=="shadowTextureSize") {
                size_t v = check_t<size_t>(L,3);
                self.setShadowTextureSize(v);
        } else if (key=="shadowDirLightTextureOffset") {
                Ogre::Real v = luaL_checknumber(L,3);
                self.setShadowDirLightTextureOffset(v);
        } else if (key=="shadowTextureFadeStart") {
                Ogre::Real v = luaL_checknumber(L,3);
                self.setShadowTextureFadeStart(v);
        } else if (key=="shadowTextureFadeEnd") {
                Ogre::Real v = luaL_checknumber(L,3);
                self.setShadowTextureFadeEnd(v);
        } else if (key=="shadowTextureSelfShadow") {
                bool v = 0!=lua_toboolean(L,3);
                self.setShadowTextureSelfShadow(v);
        } else if (key=="shadowCasterRenderBackFaces") {
                bool v = 0!=lua_toboolean(L,3);
                self.setShadowCasterRenderBackFaces(v);
        } else if (key=="shadowCameraSetup") {
                Ogre::String v = luaL_checkstring(L,3);
                Ogre::ShadowCameraSetup *p;
                if (v=="DEFAULT") {
                        p = new Ogre::DefaultShadowCameraSetup();
                } else if (v=="FOCUSED") {
                        p = new Ogre::FocusedShadowCameraSetup();
                } else if (v=="LISPSM") {
                        p = new Ogre::LiSPSMShadowCameraSetup();
                } else {
                        my_lua_error(L,"Unrecognised shadow camera setup: "
                                        + v);
                }
                self.setShadowCameraSetup(Ogre::ShadowCameraSetupPtr(p));
        } else if (key=="shadowUseInfiniteFarPlane") {
                bool v = 0!=lua_toboolean(L,3);
                self.setShadowUseInfiniteFarPlane(v);

        } else {
                my_lua_error(L,"Not a valid SceneManager member: "+key);
        }
        return 1;
TRY_END
}

EQ_PTR_MACRO(Ogre::SceneManager,scnmgr,SCNMGR_TAG)

MT_MACRO_NEWINDEX(scnmgr);

//}}}


// vim: shiftwidth=8:tabstop=8:expandtab
