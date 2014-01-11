/*
	This file is part of the MinSG library.
	Copyright (C) 2014 Claudius J�hn <claudius@uni-paderborn.de>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifdef MINSG_EXT_VOXEL_WORLD
#include "VoxelWorld.h"

#include <Rendering/Mesh/Mesh.h>
#include <Rendering/MeshUtils/MeshBuilder.h>
#include <Geometry/VoxelStorage.h>
#include <Util/Graphics/Color.h>
#include <iostream>

namespace MinSG {

struct VoxelGrid{
	const uint32_t wx,wy,wz,wxy;
	std::vector<uint32_t> voxels; // voxel, edgeflags, occlusion value
	VoxelGrid(uint32_t _wx, uint32_t _wy, uint32_t _wz) : 
		wx(_wx), wy(_wy), wz(_wz), wxy(wx*wy), voxels(wx*wy*wz){
	}
	
	void set(uint32_t x,uint32_t y,uint32_t z,uint32_t value){
		voxels[ x+y*wx+z*wxy ] = value;
	}
	inline uint32_t get(int32_t x,int32_t y,int32_t z)const{
		return (x<0||x>=static_cast<int32_t>(wx)||y<0||y>=static_cast<int32_t>(wy)||z<0||z>=static_cast<int32_t>(wz)) ? 0 : voxels[ x+y*wx+z*wxy ];
	}
	inline uint32_t get(const Geometry::_Vec3<int32_t>& v)const{
		return get(v.x(),v.y(),v.z());
	}
	uint32_t index(const Geometry::_Vec3<int32_t>& v)const{
		return v.x()+v.y()*wx+v.z()*wxy;
	}
	Geometry::_Vec3<int32_t> clamp(const Geometry::_Vec3<int32_t>& v)const{
		return Geometry::_Vec3<int32_t>(
				std::max( std::min(v.x(),static_cast<int32_t>(wx)),0 ),
				std::max( std::min(v.y(),static_cast<int32_t>(wy)),0 ),
				std::max( std::min(v.z(),static_cast<int32_t>(wz)),0 )
		);
	}
	uint32_t getSpace(int32_t x,int32_t y,int32_t z,int32_t dx,int32_t dy,int32_t dz)const{
		int32_t space = 0;
		if(get(x,y,z)==0){
			++space;
			int32_t xt = x+dx;
			for(int32_t i=1; i<4&&get(xt,y,z)==0 ;++i,xt+=dx)
				++space;
			int32_t yt = y+dy;
			for(int32_t i=1; i<4&&get(x,yt,z)==0 ;++i,yt+=dy)
				++space;
			int32_t zt = z+dz;
			for(int32_t i=1; i<4&&get(x,y,zt)==0 ;++i,zt+=dz)
				++space;
			xt = x+dx, yt = y+dy, zt = z+dz;
			for(int32_t i=1; i<4&&get(xt,yt,zt)==0 ;++i,xt+=dx,yt+=dy,zt+=dz)
				++space;
		}
		return space;
	}
	uint32_t getFreeSpace(int32_t x,int32_t y,int32_t z){
		uint32_t space = 0;
		
		space += getSpace(x,  y,  z,	 1, 1, 1);
		space += getSpace(x,  y,  z-1,	 1, 1,-1);
		space += getSpace(x,  y-1,z,	 1,-1, 1);
		space += getSpace(x,  y-1,z-1,	 1,-1,-1);
		space += getSpace(x-1,y,  z,	-1, 1, 1);
		space += getSpace(x-1,y,  z-1,	-1, 1,-1);
		space += getSpace(x-1,y-1,z,	-1,-1, 1);
		space += getSpace(x-1,y-1,z-1,	-1,-1,-1);
		return space;
	}
	
	float cast(const Geometry::Vec3& source, const Geometry::Vec3& target)const{
		Geometry::Vec3 dir = target-source;
		const float distance=dir.length();
		if(distance==0)
			return -1;
		dir/=distance;
		
		const float stepSize = 0.13;
		const Geometry::Vec3 step(dir*stepSize);
		
		Geometry::Vec3 v = source + dir*0.01f;
		int32_t x=source.x()-10/*arbitrary invalid value*/, y=0, z=0;
		
		for(float currentRayLength = 0.01f; currentRayLength<distance; currentRayLength+=stepSize){
			if(static_cast<int32_t>(v.x())!=x || static_cast<int32_t>(v.y())!=y || static_cast<int32_t>(v.z())!=z){
				x = static_cast<int32_t>(v.x());
				y = static_cast<int32_t>(v.y());
				z = static_cast<int32_t>(v.z());
				if(get(x,y,z)!=0 ){
					return -currentRayLength;
				}
			}
			v += step;
		}
		return distance;
		
	}
};


static void createVertex(Rendering::MeshUtils::MeshBuilder&mb, VoxelGrid& grid,int32_t x,int32_t y,int32_t z, const Geometry::Vec3& normal){
//	mb.color( grid.getOcclusionColor(x,y,z) );
	static const Geometry::Vec3 light( 7.49,7.57,7.55);
	const float s = grid.getFreeSpace(x,y,z)/128.0f;
	
	const Geometry::Vec3 pos(x,y,z);
	float l = -1;
	if( normal.dot( light-pos )>0)
		l = grid.cast(pos,light);
//	float l = grid.cast(pos+normal*0.05,light);
//	if(l<0){
//		const uint32_t s = getFreeSpace(x,y,z);
//		mb.color( Util::Color4ub(s,s,s,255) );
//	}
//		
//	else
//		mb.color( Util::Color4f( 1.0/l,0,0,1.0  ) );
//	mb.color( Util::Color4f( s+std::max(0.0f, 1.0f/l),s,s,1.0  ) );
	mb.color( Util::Color4f( s+std::max(0.0f,2.0f/l),s,s,1.0  ) );
	mb.position(Geometry::Vec3(x,y,z));
	mb.addVertex();

}

Util::Reference<Rendering::Mesh> VoxelWorld::generateMesh( const simpleVoxelStorage_t& voxelStorage, const Geometry::_Box<int32_t>& boundary){
	const auto data=voxelStorage.serialize(boundary);

	Rendering::MeshUtils::MeshBuilder mb;
//	for(const auto & area : data.first){
//		const auto sidelength = std::get<1>(area);
//		Rendering::MeshUtils::MeshBuilder::addBox(mb,Geometry::Box(	Geometry::Vec3(std::get<0>(area)), 
//																	Geometry::Vec3(sidelength,sidelength,sidelength)));
//	}
//	const Geometry::Vec3 unit(1,1,1);
//	for(const auto & voxels : data.second){
//		const Geometry::Vec3 origin( std::get<0>(voxels) );
//		const auto block = std::get<1>(voxels);
//		for(uint32_t i=0;i<simpleVoxelStorage_t::blockSize;++i){
//			const uint32_t value = block[i];
//			if(value!=0){
//				const auto pos = origin + Geometry::Vec3(	i%simpleVoxelStorage_t::blockSideLength,
//															(i/simpleVoxelStorage_t::blockSideLength)%simpleVoxelStorage_t::blockSideLength,
//															(i/(simpleVoxelStorage_t::blockSideLength*simpleVoxelStorage_t::blockSideLength))%simpleVoxelStorage_t::blockSideLength);
//				Rendering::MeshUtils::MeshBuilder::addBox(mb,Geometry::Box(Geometry::Box(pos,pos+unit)));
//			}
//		}
//	}
	
	VoxelGrid grid(boundary.getExtentX(),boundary.getExtentY(),boundary.getExtentZ());
	
	// fill uniform blocks
	for(const auto & area : data.first){
		const auto sidelength = std::get<1>(area);
		const auto min = grid.clamp( std::get<0>(area)-boundary.getMin() );
		const auto max = grid.clamp( std::get<0>(area)-boundary.getMin()+Geometry::_Vec3<int32_t>(sidelength,sidelength,sidelength) );
		const auto value = std::get<2>(area);
				
		for(int32_t z=min.z(); z<max.z(); ++z){
			for(int32_t y=min.y(); y<max.y(); ++y){
				for(int32_t x=min.x(); x<max.x(); ++x){
					grid.set(x,y,z,value);
				}
			}
		}
	}
	// fill leaf node blocks
	for(const auto & leafNode : data.second){
		const auto block = std::get<1>(leafNode);
		const uint32_t startIndex = grid.index( std::get<0>(leafNode)- boundary.getMin() );

		for(uint32_t blockIndex=0; blockIndex<simpleVoxelStorage_t::blockSize; ++blockIndex){
			const uint32_t value = block[blockIndex];
			if(value!=0){
				const uint32_t voxelIndex = startIndex + blockIndex%simpleVoxelStorage_t::blockSideLength +
											grid.wx * ((blockIndex/simpleVoxelStorage_t::blockSideLength)%simpleVoxelStorage_t::blockSideLength) +
											grid.wxy * ((blockIndex/(simpleVoxelStorage_t::blockSideLength*simpleVoxelStorage_t::blockSideLength))%simpleVoxelStorage_t::blockSideLength);
				if(voxelIndex<grid.voxels.size())
					grid.voxels[voxelIndex] = value;
			}
		}
	}


	const Geometry::Vec3 unit(1,1,1);
	
	mb.color( Util::Color4f(0.5,0.5,0.5) );
//	for(uint32_t z=0; z<grid.wz; ++z){
//		for(uint32_t y=0; y<grid.wy; ++y){
//			uint32_t i = y*grid.wx + grid.wxy*z;
//			for(uint32_t x=0; x<grid.wx; ++x,++i){
//				const uint32_t value = grid.voxels[i];
//				if(value!=0){
//					const Geometry::Vec3 pos(x,y,z);
//					Rendering::MeshUtils::MeshBuilder::addBox(mb,Geometry::Box(Geometry::Box(pos,pos+unit)));
//				}
//			}
//		}
//	}
	for(uint32_t z=0; z<grid.wz; ++z){
		for(uint32_t y=0; y<grid.wy; ++y){
			for(uint32_t x=0; x<grid.wx; ++x){
				const uint32_t value = grid.get(x,y,z);
				if(value!=0)
					continue;
//				plate(x,y,z,-1,0,0)
				if(grid.get(x-1,y,z)!=0){
					static const Geometry::Vec3f normal(1.0f,0,0);
					const uint32_t idx = mb.getNextIndex();
					mb.normal( normal );
					createVertex(mb, grid, x  ,y  ,z  ,normal);
					createVertex(mb, grid, x  ,y+1,z  ,normal);
					createVertex(mb, grid, x  ,y+1,z+1,normal);
					createVertex(mb, grid, x  ,y  ,z+1,normal);
					mb.addQuad(idx,idx+1,idx+2,idx+3);
				}
				if(grid.get(x+1,y,z)!=0){
					static const Geometry::Vec3f normal(-1.0f,0,0);
					const uint32_t idx = mb.getNextIndex();
					mb.normal( normal );
					createVertex(mb, grid, x+1,y  ,z  ,normal);
					createVertex(mb, grid, x+1,y  ,z+1,normal);
					createVertex(mb, grid, x+1,y+1,z+1,normal);
					createVertex(mb, grid, x+1,y+1,z  ,normal);
					mb.addQuad(idx,idx+1,idx+2,idx+3);
				}
				if(grid.get(x,y+1,z)!=0){
					static const Geometry::Vec3f normal(0,-1.0f,0);
					const uint32_t idx = mb.getNextIndex();
					mb.normal( normal );
					createVertex(mb, grid, x  ,y+1,z  ,normal);
					createVertex(mb, grid, x+1,y+1,z  ,normal);
					createVertex(mb, grid, x+1,y+1,z+1,normal);
					createVertex(mb, grid, x  ,y+1,z+1,normal);
					mb.addQuad(idx,idx+1,idx+2,idx+3);
				}
				if(grid.get(x,y-1,z)!=0){
					static const Geometry::Vec3f normal(0,1.0f,0);
					const uint32_t idx = mb.getNextIndex();
					mb.normal( normal );
					createVertex(mb, grid, x  ,y  ,z  ,normal);
					createVertex(mb, grid, x  ,y  ,z+1,normal);
					createVertex(mb, grid, x+1,y  ,z+1,normal);
					createVertex(mb, grid, x+1,y  ,z  ,normal);
					mb.addQuad(idx,idx+1,idx+2,idx+3);
				}
				if(grid.get(x,y,z+1)!=0){
					static const Geometry::Vec3f normal(0,0,-1.0f);
					const uint32_t idx = mb.getNextIndex();
					mb.normal( normal );
					createVertex(mb, grid, x  ,y  ,z+1,normal);
					createVertex(mb, grid, x  ,y+1,z+1,normal);
					createVertex(mb, grid, x+1,y+1,z+1,normal);
					createVertex(mb, grid, x+1,y  ,z+1,normal);
					mb.addQuad(idx,idx+1,idx+2,idx+3);
				}
				if(grid.get(x,y,z-1)!=0){
					static const Geometry::Vec3f normal(0,0,1.0f);
					const uint32_t idx = mb.getNextIndex();
					mb.normal( normal );
					createVertex(mb, grid, x  ,y  ,z  ,normal);
					createVertex(mb, grid, x+1,y  ,z  ,normal);
					createVertex(mb, grid, x+1,y+1,z  ,normal);
					createVertex(mb, grid, x  ,y+1,z  ,normal);
					mb.addQuad(idx,idx+1,idx+2,idx+3);
				}
			}
		}
	}
	
	return mb.buildMesh();

}


}
#endif /* MINSG_EXT_VOXEL_WORLD */