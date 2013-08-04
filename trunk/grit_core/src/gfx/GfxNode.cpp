/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "gfx_internal.h"

#include "GfxNode.h"
#include "GfxFertileNode.h"
#include "GfxBody.h"

const std::string GfxNode::className = "GfxNode";

GfxNode::GfxNode (const GfxNodePtr &par_)
{
    dead = false;

    localTransform = Transform(Vector3(0,0,0), Quaternion(1,0,0,0), Vector3(1,1,1));

    //dirtyWorldTransform = false;
    worldTransform = localTransform;

    node = ogre_sm->createSceneNode();
    ogre_root_node->addChild(node);

    par = GfxNodePtr(NULL);
    setParent(par_);

    gfx_all_nodes.push_back(this);
}

GfxNode::~GfxNode (void)
{
    if (!dead) destroy();
}
    
void GfxNode::destroy (void)
{
    setParent(GfxNodePtr(NULL));

    dead = true;

    ogre_sm->destroySceneNode(node->getName());
    node = NULL; 

    gfx_all_nodes.erase(this);
}       
    
void GfxNode::notifyParentDead (void)
{
    ensureAlive();
    APP_ASSERT(!par.isNull());
    par = GfxNodePtr(NULL);
    updateParentBoneId();
}   
    
void GfxNode::setParent (const GfxNodePtr &par_)
{
    ensureAlive();
    if (par_ == par) return;
    if (!par.isNull()) {
        par->notifyLostChild(this);
    }
    par = par_;
    if (!par.isNull()) {
        par->notifyGainedChild(this);
    }
    updateParentBoneId();
}

/* Needs to be run:
 * when parent is changed
 * when parent is reloaded (skeleton can change)
 * when parentBoneName is changed
 */
void GfxNode::updateParentBoneId (void)
{
    if (parentBoneName.length()==0 || par.isNull()) {
        parentBoneId = -1;
    } else {
        GfxBody *pbody = dynamic_cast<GfxBody*>(&*par);
        if (pbody != NULL && pbody->hasBones() && pbody->hasBoneName(parentBoneName)) {
            parentBoneId = pbody->getBoneId(parentBoneName);
        } else {
            parentBoneId = -1;
        }
    }
    //dirtyWorldTransform = true;
}
void GfxNode::doUpdateWorldTransform (void)
{
    if (par.isNull()) {
        worldTransform = localTransform;
    } else {
        GfxFertileNode *fertile_node = static_cast<GfxFertileNode*>(&*par);
        fertile_node->updateWorldTransform();
        Transform parentTransform;
        if (parentBoneId < 0) {
            parentTransform = fertile_node->getWorldTransform();
        } else {
            GfxBody *pbody = static_cast<GfxBody*>(fertile_node);
            parentTransform = pbody->getBoneWorldTransform(parentBoneId);
        }
        worldTransform = parentTransform * localTransform;
    }
    node->setPosition(to_ogre(worldTransform.p));
    node->setOrientation(to_ogre(worldTransform.r));
    node->setScale(to_ogre(worldTransform.s));
    //dirtyWorldTransform = false;
}


void GfxNode::ensureNotChildOf (GfxFertileNode *leaf) const
{
    if (leaf==this) GRIT_EXCEPT("Parenthood must be acyclic.");
    if (par.isNull()) return;
    par->ensureNotChildOf(leaf);
}
