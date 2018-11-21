//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "scene_private.h"
#include "device_private.h"
#include "rigid_private.h"

namespace flame
{
	namespace physics
	{
		void pxCallback::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
		{

		}

		void pxCallback::onWake(PxActor** actors, PxU32 count)
		{

		}

		void pxCallback::onSleep(PxActor** actors, PxU32 count)
		{

		}

		void pxCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
		{

		}

		void pxCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
		{
			if (!s->_priv->trigger_callback)
				return;

			for (auto i = 0; i < count; i++)
			{
				s->_priv->trigger_callback((Rigid*)pairs[i].triggerActor->userData, (Shape*)pairs[i].triggerShape->userData,
					(Rigid*)pairs[i].otherActor->userData, (Shape*)pairs[i].otherShape->userData, 
					(pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_FOUND) ? TouchFound : TouchLost);
			}
		}

		void pxCallback::onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
		{

		}

		void Scene::add_rigid(Rigid *r)
		{
			_priv->v->addActor(*r->_priv->v);
		}

		void Scene::remove_rigid(Rigid *r)
		{
			_priv->v->removeActor(*r->_priv->v);
		}
		
		void Scene::update(float disp)
		{
			_priv->v->simulate(disp);
			_priv->v->fetchResults(true);
		}

		void Scene::enable_callback()
		{
			_priv->v->setSimulationEventCallback(&_priv->callback);
		}

		void Scene::disable_callback()
		{
			_priv->v->setSimulationEventCallback(nullptr);
		}

		void Scene::set_trigger_callback(const TriggerCallback &callback)
		{
			_priv->trigger_callback = callback;
		}

		Scene *create_scene(Device *d, float gravity, int thread_count)
		{
			auto s = new Scene;
			
			s->_priv = new ScenePrivate;
			PxSceneDesc desc(d->_priv->inst->getTolerancesScale());
			desc.gravity = PxVec3(0.0f, gravity, 0.0f);
			desc.cpuDispatcher = PxDefaultCpuDispatcherCreate(thread_count);
			desc.filterShader = PxDefaultSimulationFilterShader;
			s->_priv->v = d->_priv->inst->createScene(desc);
			s->_priv->callback.s = s;

			return s;
		}

		void destroy_scene(Scene *s)
		{
			s->_priv->v->release();

			delete s->_priv;
			delete s;
		}
	}
}

