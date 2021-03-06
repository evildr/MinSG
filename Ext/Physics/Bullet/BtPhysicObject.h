/*
	This file is part of the MinSG library extension Physics.
	Copyright (C) 2013 Mouns Almarrani
	Copyright (C) 2009-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2009-2013 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2009-2013 Ralf Petring <ralf@petring.net>

	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifdef MINSG_EXT_PHYSICS

#ifndef BTPHYSICOBJECT_H
#define BTPHYSICOBJECT_H

#include "../PhysicObject.h"
#include "BtConstraintObject.h"
#include "ShapeContainer.h"
#include "../../../Core/Nodes/Node.h"
#include <Util/References.h>
#include <functional>
#include <set>

class btManifoldPoint;
class btCollisionObjectWrapper;
class btRigidBody;

namespace MinSG {

namespace Physics {

class BtPhysicObject : public PhysicObject{
        Util::Reference<Node> node;
        Util::Reference<ShapeContainer> shape;	// keep a reference as long as the body exists
        std::unique_ptr<btRigidBody> body;
        std::vector<Util::Reference<BtConstraintObject>> constraints;
        Geometry::Vec3 centerOfMass;
    public:

        //! create a new physic object
        BtPhysicObject(Node * _node): node(_node){}
        BtPhysicObject(const BtPhysicObject&) = delete;
        BtPhysicObject(BtPhysicObject&&) = default;
		virtual ~BtPhysicObject();

        const Geometry::Vec3& getCenterOfMass()const	{	return centerOfMass;	}
        Node* getNode()const override					{	return node.get();	}
        btRigidBody* getRigidBody()const				{	return body.get();	}
        ShapeContainer* getShape()const					{	return shape.get();	}

        const std::vector<Util::Reference<BtConstraintObject>>& getConstraints() const { return constraints; }
        void removeConstraint(BtConstraintObject& constraint);

        void setCenterOfMass(const Geometry::Vec3& v)	{	centerOfMass = v;	}
        void setBodyAndShape(btRigidBody* _body,ShapeContainer* _shape)	{
        	body.reset(_body);
			shape = _shape;
		}
		void addConstraintObject(BtConstraintObject& constraint) { constraints.emplace_back(&constraint); }


		typedef std::function<bool (btManifoldPoint& cp, BtPhysicObject* obj0, BtPhysicObject* obj1)> contactListener_t;
		contactListener_t contactListener;

};
}
}

#endif /* BTPHYSICOBJECT_H */

#endif /* MINSG_EXT_PHYSICS */
