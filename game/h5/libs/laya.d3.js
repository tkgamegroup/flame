
(function(window,document,Laya){
	var __un=Laya.un,__uns=Laya.uns,__static=Laya.static,__class=Laya.class,__getset=Laya.getset,__newvec=Laya.__newvec;

	var Arith=laya.maths.Arith,AtlasResourceManager=laya.webgl.atlas.AtlasResourceManager;
	var BaseShader=laya.webgl.shader.BaseShader,Browser=laya.utils.Browser,Buffer=laya.webgl.utils.Buffer,Byte=laya.utils.Byte;
	var ClassUtils=laya.utils.ClassUtils,Config=Laya.Config;
	var Event=laya.events.Event,EventDispatcher=laya.events.EventDispatcher,Handler=laya.utils.Handler,Loader=laya.net.Loader;
	var LoaderManager=laya.net.LoaderManager,MathUtil=laya.maths.MathUtil,Node=laya.display.Node,Render=laya.renders.Render;
	var RenderContext=laya.renders.RenderContext,RenderSprite=laya.renders.RenderSprite,RenderState2D=laya.webgl.utils.RenderState2D;
	var Resource=laya.resource.Resource,RunDriver=laya.utils.RunDriver,Shader=laya.webgl.shader.Shader,ShaderCompile=laya.webgl.utils.ShaderCompile;
	var Sprite=laya.display.Sprite,Stat=laya.utils.Stat,StringKey=laya.utils.StringKey,Style=laya.display.css.Style;
	var Texture=laya.resource.Texture,URL=laya.net.URL,Utils=laya.utils.Utils,WebGL=laya.webgl.WebGL,WebGLContext=laya.webgl.WebGLContext;
	var WebGLContext2D=laya.webgl.canvas.WebGLContext2D;
Laya.interface('laya.d3.core.IClone');
Laya.interface('laya.d3.graphics.IVertex');
Laya.interface('laya.d3.core.render.IUpdate');
Laya.interface('laya.d3.core.scene.ITreeNode');
Laya.interface('laya.d3.core.render.IRenderable');
/**
*@private
*/


/**
*<code>Layer</code> 类用于实现层。
*/
//class laya.d3.core.Layer
var Layer=(function(){
	function Layer(){
		/**@private 编号。*/
		//this._number=0;
		/**@private 蒙版值。*/
		//this._mask=0;
		/**@private 是否显示。*/
		//this._visible=false;
		/**@private 只读,不允许修改。*/
		//this._nonRigidbodyOffset=0;
		/**@private 只读,不允许修改。*/
		//this._colliders=null;
		/**名字。*/
		//this.name=null;
		this._visible=true;
		this._nonRigidbodyOffset=0;
		this._colliders=[];
	}

	__class(Layer,'laya.d3.core.Layer');
	var __proto=Layer.prototype;
	/**
	*@private
	*/
	__proto._binarySearchIndex=function(){
		var start=0;
		var end=Layer._collsionTestList.length-1;
		var mid=0;
		while (start <=end){
			mid=Math.floor((start+end)/ 2);
			var midValue=Layer._collsionTestList[mid];
			if (midValue==this._number)
				return mid;
			else if (midValue > this._number)
			end=mid-1;
			else
			start=mid+1;
		}
		return start;
	}

	/**
	*@private
	*/
	__proto._addCollider=function(collider){
		(this._colliders.length===0)&& (Layer._collsionTestList.splice(this._binarySearchIndex(),0,this._number));
		if (collider._isRigidbody){
			this._colliders.unshift(collider);
			this._nonRigidbodyOffset++;
			}else {
			this._colliders.push(collider);
		}
	}

	/**
	*@private
	*/
	__proto._removeCollider=function(collider){
		var index=this._colliders.indexOf(collider);
		if (index < this._nonRigidbodyOffset)
			this._nonRigidbodyOffset--;
		this._colliders.splice(index,1);
		(this._colliders.length===0)&& (Layer._collsionTestList.splice(Layer._collsionTestList.indexOf(this._number),1));
	}

	/**
	*获取编号。
	*@return 编号。
	*/
	__getset(0,__proto,'number',function(){
		return this._number;
	});

	/**
	*设置是否显示。
	*@param value 是否显示。
	*/
	/**
	*获取是否显示。
	*@return 是否显示。
	*/
	__getset(0,__proto,'visible',function(){
		return this._visible;
		},function(value){
		this._visible=value;
		if (value)
			Layer._visibleLayers=Layer._visibleLayers | this.mask;
		else
		Layer._visibleLayers=Layer._visibleLayers & ~this.mask;
	});

	/**
	*获取蒙版值。
	*@return 蒙版值。
	*/
	__getset(0,__proto,'mask',function(){
		return this._mask;
	});

	/**
	*设置Layer显示层。
	*@param value 显示层。
	*/
	/**
	*获取Layer显示层。
	*@return 显示层。
	*/
	__getset(1,Layer,'visibleLayers',function(){
		return Layer._visibleLayers;
		},function(value){
		Layer._visibleLayers=value;
		for (var i=0,n=Layer._layerList.length;i < n;i++){
			var layer=Layer._layerList[i];
			layer._visible=(layer._mask & Layer._visibleLayers)!==0;
		}
	});

	Layer.__init__=function(){
		Layer._layerList.length=31;
		for (var i=0;i < 31;i++){
			var layer=new Layer();
			Layer._layerList[i]=layer;
			if (i===0){
				layer.name="Default Layer";
				layer.visible=true;
				}else {
				layer.name="Layer-"+i;
				layer.visible=false;
			}
			layer._number=i;
			layer._mask=Math.pow(2,i);
		}
		Layer.currentCreationLayer=Layer._layerList[0];
	}

	Layer.getLayerByNumber=function(number){
		if (number < 0 || number > 30)
			throw new Error("无法返回指定Layer，该number超出范围！");
		return Layer._layerList[number];
	}

	Layer.getLayerByName=function(name){
		for (var i=0;i < 31;i++){
			if (Layer._layerList[i].name===name)
				return Layer._layerList[i];
		}
		throw new Error("无法返回指定Layer,该name不存在");
	}

	Layer.isVisible=function(mask){
		return (mask & Layer._currentCameraCullingMask & Layer._visibleLayers)!=0;
	}

	Layer._layerList=[];
	Layer._visibleLayers=2147483647;
	Layer._collsionTestList=[];
	Layer._currentCameraCullingMask=2147483647;
	Layer.maxCount=31;
	Layer.currentCreationLayer=null;
	return Layer;
})()



/**
*@private
*<code>RenderElement</code> 类用于实现渲染物体。
*/
//class laya.d3.core.render.RenderElement
var RenderElement=(function(){
	function RenderElement(){
		/**@private */
		this._id=0;
		/**@private 类型0为默认，2为DynamicBatch。*/
		this._type=0;
		/**@private 排序ID。*/
		this._mainSortID=0;
		/**@private */
		this._render=null;
		/**@private 所属Sprite3D精灵。*/
		this._sprite3D=null;
		/**@private 渲染所用材质。*/
		this._material=null;
		/**@private 渲染元素。*/
		this._renderObj=null;
		/**@private */
		this._staticBatch=null;
		//...............临时...........................
		this._tempBatchIndexStart=0;
		//TODO:
		this._tempBatchIndexEnd=0;
		/**@private */
		this._canDynamicBatch=false;
		/**当前ShaderValue。*/
		this._shaderValue=null;
		this._onPreRenderFunction=null;
		this._id=++RenderElement._uniqueIDCounter;
		this._canDynamicBatch=true;
		this._shaderValue=new ValusArray();
	}

	__class(RenderElement,'laya.d3.core.render.RenderElement');
	var __proto=RenderElement.prototype;
	/**
	*@private
	*/
	__proto.getDynamicBatchBakedVertexs=function(index){
		var byteSizeInFloat=4;
		var vb=this._renderObj._getVertexBuffer(index);
		var bakedVertexes=vb.getData().slice();
		var vertexDeclaration=vb.vertexDeclaration;
		var positionOffset=vertexDeclaration.getVertexElementByUsage(/*laya.d3.graphics.VertexElementUsage.POSITION0*/0).offset / byteSizeInFloat;
		var normalOffset=vertexDeclaration.getVertexElementByUsage(/*laya.d3.graphics.VertexElementUsage.NORMAL0*/3).offset / byteSizeInFloat;
		var transform=this._sprite3D.transform;
		var worldMatrix=transform.worldMatrix;
		var rotation=transform.rotation;
		var vertexFloatCount=vertexDeclaration.vertexStride / byteSizeInFloat;
		for (var i=0,n=bakedVertexes.length;i < n;i+=vertexFloatCount){
			var posOffset=i+positionOffset;
			var norOffset=i+normalOffset;
			Utils3D.transformVector3ArrayToVector3ArrayCoordinate(bakedVertexes,posOffset,worldMatrix,bakedVertexes,posOffset);
			Utils3D.transformVector3ArrayByQuat(bakedVertexes,norOffset,rotation,bakedVertexes,norOffset);
		}
		return bakedVertexes;
	}

	/**
	*@private
	*/
	__proto.getBakedIndices=function(){
		return this._renderObj._getIndexBuffer().getData();
	}

	/**
	*@private
	*/
	__proto._destroy=function(){
		(this._staticBatch)&& (this._staticBatch._manager._garbageCollection(this));
	}

	/**
	*获取唯一标识ID,通常用于识别。
	*/
	__getset(0,__proto,'id',function(){
		return this._id;
	});

	__getset(0,__proto,'renderObj',function(){
		return this._renderObj;
		},function(value){
		if (this._renderObj!==value){
			this._renderObj=value;
		}
	});

	RenderElement._uniqueIDCounter=0;
	return RenderElement;
})()


/**
*@private
*<code>RenderQuene</code> 类用于实现渲染队列。
*/
//class laya.d3.core.render.RenderQueue
var RenderQueue=(function(){
	function RenderQueue(scene){
		/**@private */
		this._id=0;
		/**@private */
		this._needSort=false;
		/**@private */
		this._renderElements=null;
		/**@private */
		this._renderableRenderObjects=null;
		/**@private */
		this._dynamicBatchCombineRenderElements=null;
		/**@private */
		this._finalElements=null;
		/**@private */
		this._scene=null;
		this._id=++RenderQueue._uniqueIDCounter;
		this._needSort=false;
		this._scene=scene;
		this._renderElements=[];
		this._renderableRenderObjects=[];
		this._dynamicBatchCombineRenderElements=[];
	}

	__class(RenderQueue,'laya.d3.core.render.RenderQueue');
	var __proto=RenderQueue.prototype;
	__proto._sortOpaqueFunc=function(a,b){
		if (a._render && b._render)
			return a._render._distanceForSort-b._render._distanceForSort;
		else
		return 0;
	}

	__proto._sortAlphaFunc=function(a,b){
		if (a._render && b._render)
			return b._render._distanceForSort-a._render._distanceForSort;
		else
		return 0;
	}

	/**
	*@private
	*/
	__proto._sortAlpha=function(cameraPos){
		RenderQueue._cameraPosition=cameraPos;
		this._finalElements.sort(this._sortAlphaFunc);
	}

	/**
	*@private
	*/
	__proto._sortOpaque=function(cameraPos){
		RenderQueue._cameraPosition=cameraPos;
		this._finalElements.sort(this._sortOpaqueFunc);
	}

	/**
	*@private
	*准备渲染队列。
	*@param state 渲染状态。
	*/
	__proto._preRender=function(state){
		this._finalElements=this._renderElements.concat(this._dynamicBatchCombineRenderElements);
	}

	/**
	*@private
	*渲染队列。
	*@param state 渲染状态。
	*/
	__proto._render=function(state,isTarget){
		var loopCount=Stat.loopCount;
		var scene=this._scene;
		var camera=state.camera;
		var cameraID=camera.id;
		var vbs;
		var vertexBuffer,vertexDeclaration,shader;
		var forceUploadParams=false;
		var lastStateMaterial,lastStateOwner;
		for (var i=0,n=this._finalElements.length;i < n;i++){
			var renderElement=this._finalElements[i];
			var renderObj,material,owner;
			if (renderElement._onPreRenderFunction !=null){
				renderElement._onPreRenderFunction.call(renderElement._sprite3D,state);
			}
			if (renderElement._type===0){
				state.owner=owner=renderElement._sprite3D;
				state.renderElement=renderElement;
				owner._preRenderUpdateComponents(state);
				renderObj=renderElement.renderObj,material=renderElement._material;
				if (renderObj._beforeRender(state)){
					vbs=renderObj._getVertexBuffers();
					vertexBuffer=renderObj._getVertexBuffer(0);
					vertexDeclaration=vertexBuffer.vertexDeclaration;
					shader=state._shader=material._getShader(scene._shaderDefineValue,vertexDeclaration.shaderDefineValue,owner._shaderDefineValue);
					forceUploadParams=shader.bind()|| (loopCount!==shader._uploadLoopCount);
					if (vbs){
						if (shader._uploadVertexBuffer!==vbs || forceUploadParams){
							for (var j=0;j < vbs.length;j++){
								var vb=vbs[j];
								shader.uploadAttributesX(vb.vertexDeclaration.shaderValues.data,vb);
							}
							shader._uploadVertexBuffer=vbs;
						}
						}else {
						if (shader._uploadVertexBuffer!==vertexBuffer || forceUploadParams){
							shader.uploadAttributes(vertexDeclaration.shaderValues.data,null);
							shader._uploadVertexBuffer=vertexBuffer;
						}
					}
					if (shader._uploadScene!==scene || forceUploadParams){
						shader.uploadSceneUniforms(scene._shaderValues.data);
						shader._uploadScene=scene;
					}
					if (camera!==shader._uploadCamera || shader._uploadSprite3D!==owner || forceUploadParams){
						shader.uploadSpriteUniforms(owner._shaderValues.data);
						shader._uploadSprite3D=owner;
					}
					if (camera!==shader._uploadCamera || forceUploadParams){
						shader.uploadCameraUniforms(camera._shaderValues.data);
						shader._uploadCamera=camera;
					}
					if (shader._uploadMaterial!==material || forceUploadParams){
						material._upload();
						shader._uploadMaterial=material;
					}
					if (lastStateMaterial!==material){
						material._setRenderStateBlendDepth();
						material._setRenderStateFrontFace(isTarget,owner.transform);
						lastStateMaterial=material;
						lastStateOwner=owner;
						}else {
						if (lastStateOwner!==owner){
							material._setRenderStateFrontFace(isTarget,owner.transform);
							lastStateOwner=owner;
						}
					}
					renderObj._render(state);
					shader._uploadLoopCount=loopCount;
				}
				owner._postRenderUpdateComponents(state);
				}else if (renderElement._type===2){
				var dynamicBatch=renderElement.renderObj;
				state.owner=owner=renderElement._sprite3D;
				state.renderElement=renderElement;
				state._batchIndexStart=renderElement._tempBatchIndexStart;
				state._batchIndexEnd=renderElement._tempBatchIndexEnd;
				renderObj=renderElement.renderObj,material=renderElement._material;
				if (renderObj._beforeRender(state)){
					vertexBuffer=renderObj._getVertexBuffer(0);
					vertexDeclaration=vertexBuffer.vertexDeclaration;
					shader=state._shader=material._getShader(scene._shaderDefineValue,vertexDeclaration.shaderDefineValue,owner._shaderDefineValue);
					forceUploadParams=shader.bind()|| (loopCount!==shader._uploadLoopCount);
					if (shader._uploadVertexBuffer!==vertexBuffer || forceUploadParams){
						shader.uploadAttributes(vertexDeclaration.shaderValues.data,null);
						shader._uploadVertexBuffer=vertexBuffer;
					}
					if (shader._uploadScene!==scene || forceUploadParams){
						shader.uploadSceneUniforms(scene._shaderValues.data);
						shader._uploadScene=scene;
					}
					if (camera!==shader._uploadCamera || shader._uploadSprite3D!==owner || forceUploadParams){
						shader.uploadSpriteUniforms(owner._shaderValues.data);
						shader._uploadSprite3D=owner;
					}
					if (camera!==shader._uploadCamera || forceUploadParams){
						shader.uploadCameraUniforms(camera._shaderValues.data);
						shader._uploadCamera=camera;
					}
					if (shader._uploadMaterial!==material || forceUploadParams){
						material._upload();
						shader._uploadMaterial=material;
					}
					if (lastStateMaterial!==material){
						material._setRenderStateBlendDepth();
						material._setRenderStateFrontFace(isTarget,owner.transform);
						lastStateMaterial=material;
						lastStateOwner=owner;
						}else {
						if (lastStateOwner!==owner){
							material._setRenderStateFrontFace(isTarget,owner.transform);
							lastStateOwner=owner;
						}
					}
					renderObj._render(state);
					shader._uploadLoopCount=loopCount;
				}
			}
		}
	}

	/**
	*@private
	*渲染队列。
	*@param state 渲染状态。
	*/
	__proto._renderShadow=function(state,isOnePSSM){
		var loopCount=Stat.loopCount;
		var scene=this._scene;
		var camera=state.camera;
		var vertexBuffer,vertexDeclaration,shader;
		var forceUploadParams=false;
		var lastStateMaterial,lastStateOwner;
		for (var i=0,n=this._finalElements.length;i < n;i++){
			var renderElement=this._finalElements[i];
			var renderObj,material,owner;
			if (renderElement._type===0){
				state.owner=owner=renderElement._sprite3D;
				if (!isOnePSSM && (owner._projectionViewWorldUpdateCamera!==camera || owner._projectionViewWorldUpdateLoopCount!==Stat.loopCount)){
					owner._render._renderUpdate(state._projectionViewMatrix);
					owner._projectionViewWorldUpdateLoopCount=Stat.loopCount;
					owner._projectionViewWorldUpdateCamera=camera;
				}
				state.renderElement=renderElement;
				owner._preRenderUpdateComponents(state);
				renderObj=renderElement.renderObj,material=renderElement._material;
				if (renderObj._beforeRender(state)){
					vertexBuffer=renderObj._getVertexBuffer(0);
					vertexDeclaration=vertexBuffer.vertexDeclaration;
					shader=state._shader=material._getShader(scene._shaderDefineValue,vertexDeclaration.shaderDefineValue,owner._shaderDefineValue);
					forceUploadParams=shader.bind()|| (loopCount!==shader._uploadLoopCount);
					if (shader._uploadVertexBuffer!==vertexBuffer || forceUploadParams){
						shader.uploadAttributes(vertexDeclaration.shaderValues.data,null);
						shader._uploadVertexBuffer=vertexBuffer;
					}
					if (camera!==shader._uploadCamera || shader._uploadSprite3D!==owner || forceUploadParams){
						shader.uploadSpriteUniforms(owner._shaderValues.data);
						shader._uploadSprite3D=owner;
					}
					if (camera!==shader._uploadCamera || forceUploadParams){
						shader.uploadCameraUniforms(camera._shaderValues.data);
						shader._uploadCamera=camera;
					}
					if (shader._uploadMaterial!==material || forceUploadParams){
						material._upload();
						shader._uploadMaterial=material;
					}
					if (shader._uploadRenderElement!==renderElement || forceUploadParams){}
						if (lastStateMaterial!==material){
						material._setRenderStateFrontFace(false,owner.transform);
						lastStateMaterial=material;
						lastStateOwner=owner;
						}else {
						if (lastStateOwner!==owner){
							material._setRenderStateFrontFace(false,owner.transform);
							lastStateOwner=owner;
						}
					}
					renderObj._render(state);
					shader._uploadLoopCount=loopCount;
				}
				owner._postRenderUpdateComponents(state);
			}
		}
	}

	/**
	*清空队列中的渲染物体。
	*/
	__proto._clearRenderElements=function(){
		this._dynamicBatchCombineRenderElements.length=0;
		this._renderElements.length=0;
		this._needSort=true;
	}

	/**
	*添加渲染物体。
	*@param renderObj 渲染物体。
	*/
	__proto._addRenderElement=function(renderElement){
		this._renderElements.push(renderElement);
		this._needSort=true;
	}

	/**
	*添加动态批处理。
	*@param renderObj 动态批处理。
	*/
	__proto._addDynamicBatchElement=function(dynamicBatchElement){
		this._dynamicBatchCombineRenderElements.push(dynamicBatchElement);
	}

	/**
	*获取唯一标识ID(通常用于优化或识别)。
	*/
	__getset(0,__proto,'id',function(){
		return this._id;
	});

	RenderQueue._uniqueIDCounter=0;
	RenderQueue.OPAQUE=1;
	RenderQueue.TRANSPARENT=2;
	RenderQueue._cameraPosition=null;
	return RenderQueue;
})()


/**
*<code>RenderState</code> 类用于实现渲染状态。
*/
//class laya.d3.core.render.RenderState
var RenderState=(function(){
	function RenderState(){
		/**@private */
		this._staticBatch=null;
		/**@private */
		this._batchIndexStart=0;
		/**@private */
		this._batchIndexEnd=0;
		/**@private */
		this._viewMatrix=null;
		/**@private */
		this._projectionMatrix=null;
		/**@private */
		this._projectionViewMatrix=null;
		/**@private */
		this._viewport=null;
		/**@private */
		this._shader=null;
		/**距上一帧间隔时间。*/
		this.elapsedTime=NaN;
		/**当前场景。*/
		this.scene=null;
		/**当前渲染3D精灵。*/
		this.owner=null;
		/**当前渲染物体。*/
		this.renderElement=null;
		/**当前摄像机。*/
		this.camera=null;
	}

	__class(RenderState,'laya.d3.core.render.RenderState');
	RenderState.clientWidth=0;
	RenderState.clientHeight=0;
	return RenderState;
})()



/**
*...
*@author ...
*/
//class laya.d3.core.scene.SceneManager
var SceneManager=(function(){
	function SceneManager(){}
	__class(SceneManager,'laya.d3.core.scene.SceneManager');
	return SceneManager;
})()


//class laya.d3.core.trail.module.Color
var Color$1=(function(){
	function Color(r,g,b,a){
		this._r=NaN;
		this._g=NaN;
		this._b=NaN;
		this._a=NaN;
		(r===void 0)&& (r=1);
		(g===void 0)&& (g=1);
		(b===void 0)&& (b=1);
		(a===void 0)&& (a=1);
		this._r=r;
		this._g=g;
		this._b=b;
		this._a=a;
	}

	__class(Color,'laya.d3.core.trail.module.Color',null,'Color$1');
	var __proto=Color.prototype;
	__proto.cloneTo=function(destObject){
		destObject._r=this._r;
		destObject._g=this._g;
		destObject._b=this._b;
		destObject._a=this._a;
	}

	__static(Color,
	['RED',function(){return this.RED=new Color(1,0,0,1);},'GREEN',function(){return this.GREEN=new Color(0,1,0,1);},'BLUE',function(){return this.BLUE=new Color(0,0,1,1);},'CYAN',function(){return this.CYAN=new Color(0,1,1,1);},'YELLOW',function(){return this.YELLOW=new Color(1,0.92,0.016,1);},'MAGENTA',function(){return this.MAGENTA=new Color(1,0,1,1);},'GRAY',function(){return this.GRAY=new Color(0.5,0.5,0.5,1);},'WHITE',function(){return this.WHITE=new Color(1,1,1,1);},'BLACK',function(){return this.BLACK=new Color(0,0,0,1);}
	]);
	return Color;
})()


/**
*@private
*/
//class laya.d3.graphics.FrustumCulling
var FrustumCulling=(function(){
	function FrustumCulling(){}
	__class(FrustumCulling,'laya.d3.graphics.FrustumCulling');
	FrustumCulling.renderShadowObjectCulling=function(scene,lightFrustum,shadowQueues,lightViewProjectMatrix,nPSSMNum){
		var i=0,j=0,n=0,m=0;
		for (i=0,n=shadowQueues.length;i < n;i++){
			var quene=shadowQueues[i];
			(quene)&& (quene._clearRenderElements());
		};
		var frustumCullingObjects=scene._cullingRenders;
		var baseRender,shadowQueue,renderElements;
		if (nPSSMNum > 1){
			for (i=0,n=scene._cullingRendersLength;i < n;i++){
				baseRender=frustumCullingObjects[i];
				if (baseRender.castShadow && Layer.isVisible(baseRender._owner.layer.mask)&& baseRender.enable){
					for (var k=1,kNum=lightFrustum.length;k < kNum;k++){
						shadowQueue=shadowQueues[k-1];
						if (lightFrustum[k].containsBoundSphere(baseRender.boundingSphere)!==/*laya.d3.math.ContainmentType.Disjoint*/0){
							renderElements=baseRender._renderElements;
							for (j=0,m=renderElements.length;j < m;j++)
							shadowQueue._addRenderElement(renderElements[j]);
						}
					}
				}
			}
			}else {
			for (i=0,n=scene._cullingRendersLength;i < n;i++){
				baseRender=frustumCullingObjects[i];
				if (baseRender.castShadow && Layer.isVisible(baseRender._owner.layer.mask)&& baseRender.enable){
					if (lightFrustum[0].containsBoundSphere(baseRender.boundingSphere)!==/*laya.d3.math.ContainmentType.Disjoint*/0){
						baseRender._renderUpdate(lightViewProjectMatrix);
						shadowQueue=shadowQueues[0];
						renderElements=baseRender._renderElements;
						for (j=0,m=renderElements.length;j < m;j++)
						shadowQueue._addRenderElement(renderElements[j]);
					}
				}
			}
		}
	}

	FrustumCulling.renderShadowObjectCullingOctree=function(scene,lightFrustum,quenesResult,lightViewProjectMatrix,nPSSMNum){
		for (var i=0,n=quenesResult.length;i < n;i++){
			var quene=quenesResult[i];
			(quene)&& (quene._clearRenderElements());
		}
		if (nPSSMNum > 1){
			scene.treeRoot.cullingShadowObjects(lightFrustum,quenesResult,true,0,scene);
			}else {
			scene.treeRoot.cullingShadowObjectsOnePSSM(lightFrustum[0],quenesResult,lightViewProjectMatrix,true,0,scene);
		}
	}

	FrustumCulling.renderObjectCulling=function(boundFrustum,scene,camera,view,projection,projectionView){
		var i=0,n=0,j=0,m=0;
		var queues=scene._quenes;
		var frustumCullingObjects=scene._cullingRenders;
		for (i=0,n=queues.length;i < n;i++){
			var queue=queues[i];
			(queue)&& (queue._clearRenderElements());
		};
		var cameraPosition=camera.transform.position;
		for (i=0,n=scene._cullingRendersLength;i < n;i++){
			var baseRender=frustumCullingObjects[i];
			if (Layer.isVisible(baseRender._owner.layer.mask)&& baseRender.enable && (boundFrustum.containsBoundSphere(baseRender.boundingSphere)!==/*laya.d3.math.ContainmentType.Disjoint*/0)){
				if (baseRender._renderUpdate(projectionView)){
					baseRender._distanceForSort=Vector3.distance(baseRender.boundingSphere.center,cameraPosition)+baseRender.sortingFudge;
					var renderElements=baseRender._renderElements;
					for (j=0,m=renderElements.length;j < m;j++){
						var renderElement=renderElements[j];
						var staticBatch=renderElement._staticBatch;
						if (staticBatch && (staticBatch._material===renderElement._material)){
							staticBatch._addBatchRenderElement(renderElement);
							}else {
							scene.getRenderQueue(renderElement._material.renderQueue)._addRenderElement(renderElement);
						}
					}
				}
			}
		}
	}

	FrustumCulling.renderObjectCullingOctree=function(boundFrustum,scene,camera,view,projection,projectionView){
		var i=0,n=0;
		var queues=scene._quenes;
		for (i=0,n=queues.length;i < n;i++){
			var queue=queues[i];
			(queue)&& (queue._clearRenderElements());
		};
	}

	FrustumCulling.renderObjectCullingNoBoundFrustum=function(scene,camera,view,projection,projectionView){
		var i=0,n=0,j=0,m=0;
		var queues=scene._quenes;
		var frustumCullingObjects=scene._cullingRenders;
		for (i=0,n=queues.length;i < n;i++){
			var queue=queues[i];
			(queue)&& (queue._clearRenderElements());
		};
		var cameraPosition=camera.transform.position;
		for (i=0,n=scene._cullingRendersLength;i < n;i++){
			var baseRender=frustumCullingObjects[i];
			if (Layer.isVisible(baseRender._owner.layer.mask)&& baseRender.enable){
				baseRender._renderUpdate(projectionView);
				baseRender._distanceForSort=Vector3.distance(baseRender.boundingSphere.center,cameraPosition)+baseRender.sortingFudge;
				var renderElements=baseRender._renderElements;
				for (j=0,m=renderElements.length;j < m;j++){
					var renderElement=renderElements[j];
					var staticBatch=renderElement._staticBatch;
					if (staticBatch && (staticBatch._material===renderElement._material)){
						staticBatch._addBatchRenderElement(renderElement);
						}else {
						var renderObj=renderElement.renderObj;
						scene.getRenderQueue(renderElement._material.renderQueue)._addRenderElement(renderElement);
					}
				}
			}
		}
	}

	return FrustumCulling;
})()


/**
*...
*@author ...
*/
//class laya.d3.graphics.VertexDeclaration
var VertexDeclaration=(function(){
	function VertexDeclaration(vertexStride,vertexElements){
		this._id=0;
		this._shaderValues=null;
		this._shaderDefineValue=0;
		//private var _shaderAttribute:*;
		this._vertexStride=0;
		this._vertexElements=null;
		this._vertexElementsDic=null;
		this._id=++VertexDeclaration._uniqueIDCounter;
		if (this._id > VertexDeclaration.maxVertexDeclaration)
			throw new Error("VertexDeclaration: VertexDeclaration count should not large than ",VertexDeclaration.maxVertexDeclaration);
		this._shaderValues=new ValusArray();
		this._vertexElementsDic={};
		this._vertexStride=vertexStride;
		this._vertexElements=vertexElements;
		for (var i=0;i < vertexElements.length;i++){
			var vertexElement=vertexElements[i];
			var attributeName=vertexElement.elementUsage;
			this._vertexElementsDic[attributeName]=vertexElement;
			var value=VertexDeclaration._getTypeInfo(vertexElement.elementFormat);
			value.push(this._vertexStride,vertexElement.offset);
			this._shaderValues.setValue(attributeName,value);
			switch (attributeName){
				case /*laya.d3.graphics.VertexElementUsage.COLOR0*/1:
					this._addShaderDefine(ShaderCompile3D.SHADERDEFINE_COLOR);
					break
				case /*laya.d3.graphics.VertexElementUsage.TEXTURECOORDINATE0*/2:
					this._addShaderDefine(ShaderCompile3D.SHADERDEFINE_UV0);
					break ;
				case /*laya.d3.graphics.VertexElementUsage.TEXTURECOORDINATE1*/15:
					this._addShaderDefine(ShaderCompile3D.SHADERDEFINE_UV1);
					break ;
				}
		}
	}

	__class(VertexDeclaration,'laya.d3.graphics.VertexDeclaration');
	var __proto=VertexDeclaration.prototype;
	/**
	*增加Shader宏定义。
	*@param value 宏定义。
	*/
	__proto._addShaderDefine=function(value){
		this._shaderDefineValue |=value;
	}

	/**
	*移除Shader宏定义。
	*@param value 宏定义。
	*/
	__proto._removeShaderDefine=function(value){
		this._shaderDefineValue &=~value;
	}

	__proto.getVertexElements=function(){
		return this._vertexElements.slice();
	}

	__proto.getVertexElementByUsage=function(usage){
		return this._vertexElementsDic[usage];
	}

	__proto.unBinding=function(){}
	__getset(0,__proto,'shaderDefineValue',function(){
		return this._shaderDefineValue;
	});

	/**
	*获取唯一标识ID(通常用于优化或识别)。
	*@return 唯一标识ID
	*/
	__getset(0,__proto,'id',function(){
		return this._id;
	});

	__getset(0,__proto,'vertexStride',function(){
		return this._vertexStride;
	});

	__getset(0,__proto,'shaderValues',function(){
		return this._shaderValues;
	});

	VertexDeclaration._getTypeSize=function(format){
		switch (format){
			case /*laya.d3.graphics.VertexElementFormat.Single*/"single":
				return 4;
			case /*laya.d3.graphics.VertexElementFormat.Vector2*/"vector2":
				return 8;
			case /*laya.d3.graphics.VertexElementFormat.Vector3*/"vector3":
				return 12;
			case /*laya.d3.graphics.VertexElementFormat.Vector4*/"vector4":
				return 16;
			case /*laya.d3.graphics.VertexElementFormat.Color*/"color":
				return 4;
			case /*laya.d3.graphics.VertexElementFormat.Byte4*/"byte4":
				return 4;
			case /*laya.d3.graphics.VertexElementFormat.Short2*/"short2":
				return 4;
			case /*laya.d3.graphics.VertexElementFormat.Short4*/"short4":
				return 8;
			case /*laya.d3.graphics.VertexElementFormat.NormalizedShort2*/"normalizedshort2":
				return 4;
			case /*laya.d3.graphics.VertexElementFormat.NormalizedShort4*/"normalizedshort4":
				return 8;
			case /*laya.d3.graphics.VertexElementFormat.HalfVector2*/"halfvector2":
				return 4;
			case /*laya.d3.graphics.VertexElementFormat.HalfVector4*/"halfvector4":
				return 8;
			}
		return 0;
	}

	VertexDeclaration._getTypeInfo=function(format){
		switch (format){
			case /*laya.d3.graphics.VertexElementFormat.Single*/"single":
				return [1, laya.webgl.WebGLContext.FLOAT, false];
			case /*laya.d3.graphics.VertexElementFormat.Vector2*/"vector2":
				return [2, laya.webgl.WebGLContext.FLOAT, false];
			case /*laya.d3.graphics.VertexElementFormat.Vector3*/"vector3":
				return [3, laya.webgl.WebGLContext.FLOAT, false];
			case /*laya.d3.graphics.VertexElementFormat.Vector4*/"vector4":
				return [4, laya.webgl.WebGLContext.FLOAT, false];
			case /*laya.d3.graphics.VertexElementFormat.Color*/"color":
				return [4, laya.webgl.WebGLContext.UNSIGNED_BYTE, true];
			case /*laya.d3.graphics.VertexElementFormat.Byte4*/"byte4":
				return [4, laya.webgl.WebGLContext.UNSIGNED_BYTE, false];
			case /*laya.d3.graphics.VertexElementFormat.Short2*/"short2":
				return [2, laya.webgl.WebGLContext.SHORT, false];
			case /*laya.d3.graphics.VertexElementFormat.Short4*/"short4":
				return [4, laya.webgl.WebGLContext.SHORT, false];
			case /*laya.d3.graphics.VertexElementFormat.NormalizedShort2*/"normalizedshort2":
				return [2, laya.webgl.WebGLContext.SHORT, true];
			case /*laya.d3.graphics.VertexElementFormat.NormalizedShort4*/"normalizedshort4":
				return [4, laya.webgl.WebGLContext.SHORT, true];
			case /*laya.d3.graphics.VertexElementFormat.HalfVector2*/"halfvector2":
				throw( "not implemented!!!!!" );
			case /*laya.d3.graphics.VertexElementFormat.HalfVector4*/"halfvector4":
				throw( "not implemented!!!!!" );
			}
		return 0;
	}

	VertexDeclaration.getVertexStride=function(vertexElements){
		var curStride=0;
		for (var i=0;i < vertexElements.Length;i++){
			var element=vertexElements[i];
			var stride=element.offset+VertexDeclaration._getTypeSize(element.elementFormat);
			if (curStride < stride){
				curStride=stride;
			}
		}
		return curStride;
	}

	VertexDeclaration._maxVertexDeclarationBit=1000;
	VertexDeclaration._uniqueIDCounter=1;
	__static(VertexDeclaration,
	['maxVertexDeclaration',function(){return this.maxVertexDeclaration=2147483647-Math.floor(2147483647 / 1000)*1000;}
	]);
	return VertexDeclaration;
})()


/**
*<code>VertexElement</code> 类用于创建顶点结构分配。
*/
//class laya.d3.graphics.VertexElement
var VertexElement=(function(){
	function VertexElement(offset,elementFormat,elementUsage){
		this.offset=0;
		this.elementFormat=null;
		this.elementUsage=0;
		this.offset=offset;
		this.elementFormat=elementFormat;
		this.elementUsage=elementUsage;
	}

	__class(VertexElement,'laya.d3.graphics.VertexElement');
	return VertexElement;
})()


/**
*...
*@author ...
*/
//class laya.d3.graphics.VertexElementFormat
var VertexElementFormat=(function(){
	function VertexElementFormat(){}
	__class(VertexElementFormat,'laya.d3.graphics.VertexElementFormat');
	VertexElementFormat.Single="single";
	VertexElementFormat.Vector2="vector2";
	VertexElementFormat.Vector3="vector3";
	VertexElementFormat.Vector4="vector4";
	VertexElementFormat.Color="color";
	VertexElementFormat.Byte4="byte4";
	VertexElementFormat.Short2="short2";
	VertexElementFormat.Short4="short4";
	VertexElementFormat.NormalizedShort2="normalizedshort2";
	VertexElementFormat.NormalizedShort4="normalizedshort4";
	VertexElementFormat.HalfVector2="halfvector2";
	VertexElementFormat.HalfVector4="halfvector4";
	return VertexElementFormat;
})()


/**
*...
*@author ...
*/
//class laya.d3.graphics.VertexElementUsage
var VertexElementUsage=(function(){
	function VertexElementUsage(){}
	__class(VertexElementUsage,'laya.d3.graphics.VertexElementUsage');
	VertexElementUsage.POSITION0=0;
	VertexElementUsage.COLOR0=1;
	VertexElementUsage.TEXTURECOORDINATE0=2;
	VertexElementUsage.NORMAL0=3;
	VertexElementUsage.BINORMAL0=4;
	VertexElementUsage.TANGENT0=5;
	VertexElementUsage.BLENDINDICES0=6;
	VertexElementUsage.BLENDWEIGHT0=7;
	VertexElementUsage.DEPTH0=8;
	VertexElementUsage.FOG0=9;
	VertexElementUsage.POINTSIZE0=10;
	VertexElementUsage.SAMPLE0=11;
	VertexElementUsage.TESSELLATEFACTOR0=12;
	VertexElementUsage.COLOR1=13;
	VertexElementUsage.NEXTTEXTURECOORDINATE0=14;
	VertexElementUsage.TEXTURECOORDINATE1=15;
	VertexElementUsage.NEXTTEXTURECOORDINATE1=16;
	VertexElementUsage.CORNERTEXTURECOORDINATE0=17;
	VertexElementUsage.VELOCITY0=18;
	VertexElementUsage.STARTCOLOR0=19;
	VertexElementUsage.STARTSIZE=20;
	VertexElementUsage.AGEADDSCALE0=21;
	VertexElementUsage.STARTROTATION=22;
	VertexElementUsage.ENDCOLOR0=23;
	VertexElementUsage.STARTLIFETIME=24;
	VertexElementUsage.TIME0=33;
	VertexElementUsage.SHAPEPOSITIONSTARTLIFETIME=30;
	VertexElementUsage.DIRECTIONTIME=32;
	VertexElementUsage.SIZEROTATION0=27;
	VertexElementUsage.RADIUS0=28;
	VertexElementUsage.RADIAN0=29;
	VertexElementUsage.STARTSPEED=31;
	VertexElementUsage.RANDOM0=34;
	VertexElementUsage.RANDOM1=35;
	VertexElementUsage.SIMULATIONWORLDPOSTION=36;
	VertexElementUsage.SIMULATIONWORLDROTATION=37;
	VertexElementUsage.TEXTURECOORDINATE0X=38;
	VertexElementUsage.TEXTURECOORDINATE0X1=39;
	VertexElementUsage.TEXTURECOORDINATE0Y=40;
	VertexElementUsage.OFFSETVECTOR=41;
	return VertexElementUsage;
})()

/**
*<code>BoundBox</code> 类用于创建包围盒。
*/
//class laya.d3.math.BoundBox
var BoundBox=(function(){
	function BoundBox(min,max){
		/**最小顶点。*/
		this.min=null;
		/**最大顶点。*/
		this.max=null;
		this.min=min;
		this.max=max;
	}

	__class(BoundBox,'laya.d3.math.BoundBox');
	var __proto=BoundBox.prototype;
	Laya.imps(__proto,{"laya.d3.core.IClone":true})
	/**
	*获取包围盒的8个角顶点。
	*@param corners 返回顶点的输出队列。
	*/
	__proto.getCorners=function(corners){
		corners.length=8;
		var mine=this.min.elements;
		var maxe=this.max.elements;
		var minX=mine[0];
		var minY=mine[1];
		var minZ=mine[2];
		var maxX=maxe[0];
		var maxY=maxe[1];
		var maxZ=maxe[2];
		corners[0]=new Vector3(minX,maxY,maxZ);
		corners[1]=new Vector3(maxX,maxY,maxZ);
		corners[2]=new Vector3(maxX,minY,maxZ);
		corners[3]=new Vector3(minX,minY,maxZ);
		corners[4]=new Vector3(minX,maxY,minZ);
		corners[5]=new Vector3(maxX,maxY,minZ);
		corners[6]=new Vector3(maxX,minY,minZ);
		corners[7]=new Vector3(minX,minY,minZ);
	}

	__proto.toDefault=function(){
		this.min.toDefault();
		this.max.toDefault();
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		var dest=destObject;
		this.min.cloneTo(dest.min);
		this.max.cloneTo(dest.max);
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var dest=/*__JS__ */new this.constructor(new Vector3(),new Vector3());
		this.cloneTo(dest);
		return dest;
	}

	BoundBox.createfromPoints=function(points,out){
		if (points==null)
			throw new Error("points");
		var min=out.min;
		var max=out.max;
		var minE=min.elements;
		minE[0]=Number.MAX_VALUE;
		minE[1]=Number.MAX_VALUE;
		minE[2]=Number.MAX_VALUE;
		var maxE=max.elements;
		maxE[0]=-Number.MAX_VALUE;
		maxE[1]=-Number.MAX_VALUE;
		maxE[2]=-Number.MAX_VALUE;
		for (var i=0,n=points.length;i < n;++i){
			Vector3.min(min,points[i],min);
			Vector3.max(max,points[i],max);
		}
	}

	BoundBox.merge=function(box1,box2,out){
		Vector3.min(box1.min,box2.min,out.min);
		Vector3.max(box1.max,box2.max,out.max);
	}

	return BoundBox;
})()


/**
*<code>BoundFrustum</code> 类用于创建锥截体。
*/
//class laya.d3.math.BoundFrustum
var BoundFrustum=(function(){
	function BoundFrustum(matrix){
		/**4x4矩阵*/
		this._matrix=null;
		/**近平面*/
		this._near=null;
		/**远平面*/
		this._far=null;
		/**左平面*/
		this._left=null;
		/**右平面*/
		this._right=null;
		/**顶平面*/
		this._top=null;
		/**底平面*/
		this._bottom=null;
		this._matrix=matrix;
		this._near=new Plane(new Vector3());
		this._far=new Plane(new Vector3());
		this._left=new Plane(new Vector3());
		this._right=new Plane(new Vector3());
		this._top=new Plane(new Vector3());
		this._bottom=new Plane(new Vector3());
		BoundFrustum._getPlanesFromMatrix(this._matrix,this._near,this._far,this._left,this._right,this._top,this._bottom);
	}

	__class(BoundFrustum,'laya.d3.math.BoundFrustum');
	var __proto=BoundFrustum.prototype;
	/**
	*判断是否与其他锥截体相等。
	*@param other 锥截体。
	*/
	__proto.equalsBoundFrustum=function(other){
		return this._matrix.equalsOtherMatrix(other.matrix)
	}

	/**
	*判断是否与其他对象相等。
	*@param obj 对象。
	*/
	__proto.equalsObj=function(obj){
		if ((obj instanceof laya.d3.math.BoundFrustum )){
			var bf=obj;
			return this.equalsBoundFrustum(bf);
		}
		return false;
	}

	/**
	*获取锥截体的任意一平面。
	*0:近平面
	*1:远平面
	*2:左平面
	*3:右平面
	*4:顶平面
	*5:底平面
	*@param index 索引。
	*/
	__proto.getPlane=function(index){
		switch (index){
			case 0:
				return this._near;
			case 1:
				return this._far;
			case 2:
				return this._left;
			case 3:
				return this._right;
			case 4:
				return this._top;
			case 5:
				return this._bottom;
			default :
				return null;
			}
	}

	/**
	*锥截体的8个顶点。
	*@param corners 返回顶点的输出队列。
	*/
	__proto.getCorners=function(corners){
		BoundFrustum._get3PlaneInterPoint(this._near,this._bottom,this._right).cloneTo(corners[0]);
		BoundFrustum._get3PlaneInterPoint(this._near,this._top,this._right).cloneTo(corners[1]);
		BoundFrustum._get3PlaneInterPoint(this._near,this._top,this._left).cloneTo(corners[2]);
		BoundFrustum._get3PlaneInterPoint(this._near,this._bottom,this._left).cloneTo(corners[3]);
		BoundFrustum._get3PlaneInterPoint(this._far,this._bottom,this._right).cloneTo(corners[4]);
		BoundFrustum._get3PlaneInterPoint(this._far,this._top,this._right).cloneTo(corners[5]);
		BoundFrustum._get3PlaneInterPoint(this._far,this._top,this._left).cloneTo(corners[6]);
		BoundFrustum._get3PlaneInterPoint(this._far,this._bottom,this._left).cloneTo(corners[7]);
	}

	/**
	*与点的位置关系。返回-1,包涵;0,相交;1,不相交
	*@param point 点。
	*/
	__proto.containsPoint=function(point){
		var result=Plane.PlaneIntersectionType_Front;
		var planeResult=Plane.PlaneIntersectionType_Front;
		for (var i=0;i < 6;i++){
			switch (i){
				case 0:
					planeResult=Collision.intersectsPlaneAndPoint(this._near,point);
					break ;
				case 1:
					planeResult=Collision.intersectsPlaneAndPoint(this._far,point);
					break ;
				case 2:
					planeResult=Collision.intersectsPlaneAndPoint(this._left,point);
					break ;
				case 3:
					planeResult=Collision.intersectsPlaneAndPoint(this._right,point);
					break ;
				case 4:
					planeResult=Collision.intersectsPlaneAndPoint(this._top,point);
					break ;
				case 5:
					planeResult=Collision.intersectsPlaneAndPoint(this._bottom,point);
					break ;
				}
			switch (planeResult){
				case Plane.PlaneIntersectionType_Back:
					return /*laya.d3.math.ContainmentType.Disjoint*/0;
				case Plane.PlaneIntersectionType_Intersecting:
					result=Plane.PlaneIntersectionType_Intersecting;
					break ;
				}
		}
		switch (result){
			case Plane.PlaneIntersectionType_Intersecting:
				return /*laya.d3.math.ContainmentType.Intersects*/2;
			default :
				return /*laya.d3.math.ContainmentType.Contains*/1;
			}
	}

	/**
	*与包围盒的位置关系。返回-1,包涵;0,相交;1,不相交
	*@param box 包围盒。
	*/
	__proto.containsBoundBox=function(box){
		var p=BoundFrustum._tempV30,n=BoundFrustum._tempV31;
		var plane;
		var result=/*laya.d3.math.ContainmentType.Contains*/1;
		for (var i=0;i < 6;i++){
			plane=this.getPlane(i);
			this._getBoxToPlanePVertexNVertex(box,plane.normal,p,n);
			if (Collision.intersectsPlaneAndPoint(plane,p)===Plane.PlaneIntersectionType_Back)
				return /*laya.d3.math.ContainmentType.Disjoint*/0;
			if (Collision.intersectsPlaneAndPoint(plane,n)===Plane.PlaneIntersectionType_Back)
				result=/*laya.d3.math.ContainmentType.Intersects*/2;
		}
		return result;
	}

	/**
	*与包围球的位置关系。返回-1,包涵;0,相交;1,不相交
	*@param sphere 包围球。
	*/
	__proto.containsBoundSphere=function(sphere){
		var result=Plane.PlaneIntersectionType_Front;
		var planeResult=Plane.PlaneIntersectionType_Front;
		for (var i=0;i < 6;i++){
			switch (i){
				case 0:
					planeResult=Collision.intersectsPlaneAndSphere(this._near,sphere);
					break ;
				case 1:
					planeResult=Collision.intersectsPlaneAndSphere(this._far,sphere);
					break ;
				case 2:
					planeResult=Collision.intersectsPlaneAndSphere(this._left,sphere);
					break ;
				case 3:
					planeResult=Collision.intersectsPlaneAndSphere(this._right,sphere);
					break ;
				case 4:
					planeResult=Collision.intersectsPlaneAndSphere(this._top,sphere);
					break ;
				case 5:
					planeResult=Collision.intersectsPlaneAndSphere(this._bottom,sphere);
					break ;
				}
			switch (planeResult){
				case Plane.PlaneIntersectionType_Back:
					return /*laya.d3.math.ContainmentType.Disjoint*/0;
				case Plane.PlaneIntersectionType_Intersecting:
					result=Plane.PlaneIntersectionType_Intersecting;
					break ;
				}
		}
		switch (result){
			case Plane.PlaneIntersectionType_Intersecting:
				return /*laya.d3.math.ContainmentType.Intersects*/2;
			default :
				return /*laya.d3.math.ContainmentType.Contains*/1;
			}
	}

	/**
	*@private
	*/
	__proto._getBoxToPlanePVertexNVertex=function(box,planeNormal,outP,outN){
		var boxMin=box.min;
		var boxMinE=boxMin.elements;
		var boxMax=box.max;
		var boxMaxE=boxMax.elements;
		var planeNorE=planeNormal.elements;
		var planeNorEX=planeNorE[0];
		var planeNorEY=planeNorE[1];
		var planeNorEZ=planeNorE[2];
		boxMin.cloneTo(outP);;
		var outPE=outP.elements;
		if (planeNorEX >=0)
			outPE[0]=boxMaxE[0];
		if (planeNorEY >=0)
			outPE[1]=boxMaxE[1];
		if (planeNorEZ >=0)
			outPE[2]=boxMaxE[2];
		boxMax.cloneTo(outN);
		var outNE=outN.elements;
		if (planeNorEX >=0)
			outNE[0]=boxMinE[0];
		if (planeNorEY >=0)
			outNE[1]=boxMinE[1];
		if (planeNorEZ >=0)
			outNE[2]=boxMinE[2];
	}

	/**
	*获取顶平面。
	*@return 顶平面。
	*/
	__getset(0,__proto,'top',function(){
		return this._top;
	});

	/**
	*设置描述矩阵。
	*@param matrix 描述矩阵。
	*/
	/**
	*获取描述矩阵。
	*@return 描述矩阵。
	*/
	__getset(0,__proto,'matrix',function(){
		return this._matrix;
		},function(matrix){
		this._matrix=matrix;
		BoundFrustum._getPlanesFromMatrix(this._matrix,this._near,this._far,this._left,this._right,this._top,this._bottom);
	});

	/**
	*获取近平面。
	*@return 近平面。
	*/
	__getset(0,__proto,'near',function(){
		return this._near;
	});

	/**
	*获取远平面。
	*@return 远平面。
	*/
	__getset(0,__proto,'far',function(){
		return this._far;
	});

	/**
	*获取左平面。
	*@return 左平面。
	*/
	__getset(0,__proto,'left',function(){
		return this._left;
	});

	/**
	*获取右平面。
	*@return 右平面。
	*/
	__getset(0,__proto,'right',function(){
		return this._right;
	});

	/**
	*获取底平面。
	*@return 底平面。
	*/
	__getset(0,__proto,'bottom',function(){
		return this._bottom;
	});

	BoundFrustum._getPlanesFromMatrix=function(m,np,fp,lp,rp,tp,bp){
		var matrixE=m.elements;
		var m11=matrixE[0];
		var m12=matrixE[1];
		var m13=matrixE[2];
		var m14=matrixE[3];
		var m21=matrixE[4];
		var m22=matrixE[5];
		var m23=matrixE[6];
		var m24=matrixE[7];
		var m31=matrixE[8];
		var m32=matrixE[9];
		var m33=matrixE[10];
		var m34=matrixE[11];
		var m41=matrixE[12];
		var m42=matrixE[13];
		var m43=matrixE[14];
		var m44=matrixE[15];
		var nearNorE=np.normal.elements;
		nearNorE[0]=m14+m13;
		nearNorE[1]=m24+m23;
		nearNorE[2]=m34+m33;
		np.distance=m44+m43;
		np.normalize();
		var farNorE=fp.normal.elements;
		farNorE[0]=m14-m13;
		farNorE[1]=m24-m23;
		farNorE[2]=m34-m33;
		fp.distance=m44-m43;
		fp.normalize();
		var leftNorE=lp.normal.elements;
		leftNorE[0]=m14+m11;
		leftNorE[1]=m24+m21;
		leftNorE[2]=m34+m31;
		lp.distance=m44+m41;
		lp.normalize();
		var rightNorE=rp.normal.elements;
		rightNorE[0]=m14-m11;
		rightNorE[1]=m24-m21;
		rightNorE[2]=m34-m31;
		rp.distance=m44-m41;
		rp.normalize();
		var topNorE=tp.normal.elements;
		topNorE[0]=m14-m12;
		topNorE[1]=m24-m22;
		topNorE[2]=m34-m32;
		tp.distance=m44-m42;
		tp.normalize();
		var bottomNorE=bp.normal.elements;
		bottomNorE[0]=m14+m12;
		bottomNorE[1]=m24+m22;
		bottomNorE[2]=m34+m32;
		bp.distance=m44+m42;
		bp.normalize();
	}

	BoundFrustum._get3PlaneInterPoint=function(p1,p2,p3){
		var p1Nor=p1.normal;
		var p2Nor=p2.normal;
		var p3Nor=p3.normal;
		Vector3.cross(p2Nor,p3Nor,BoundFrustum._tempV30);
		Vector3.cross(p3Nor,p1Nor,BoundFrustum._tempV31);
		Vector3.cross(p1Nor,p2Nor,BoundFrustum._tempV32);
		var a=Vector3.dot(p1Nor,BoundFrustum._tempV30);
		var b=Vector3.dot(p2Nor,BoundFrustum._tempV31);
		var c=Vector3.dot(p3Nor,BoundFrustum._tempV32);
		Vector3.scale(BoundFrustum._tempV30,-p1.distance / a,BoundFrustum._tempV33);
		Vector3.scale(BoundFrustum._tempV31,-p2.distance / b,BoundFrustum._tempV34);
		Vector3.scale(BoundFrustum._tempV32,-p3.distance / c,BoundFrustum._tempV35);
		Vector3.add(BoundFrustum._tempV33,BoundFrustum._tempV34,BoundFrustum._tempV36);
		Vector3.add(BoundFrustum._tempV35,BoundFrustum._tempV36,BoundFrustum._tempV37);
		var v=BoundFrustum._tempV37;
		return v;
	}

	__static(BoundFrustum,
	['_tempV30',function(){return this._tempV30=new Vector3();},'_tempV31',function(){return this._tempV31=new Vector3();},'_tempV32',function(){return this._tempV32=new Vector3();},'_tempV33',function(){return this._tempV33=new Vector3();},'_tempV34',function(){return this._tempV34=new Vector3();},'_tempV35',function(){return this._tempV35=new Vector3();},'_tempV36',function(){return this._tempV36=new Vector3();},'_tempV37',function(){return this._tempV37=new Vector3();}
	]);
	return BoundFrustum;
})()


/**
*<code>BoundSphere</code> 类用于创建包围球。
*/
//class laya.d3.math.BoundSphere
var BoundSphere=(function(){
	function BoundSphere(center,radius){
		/**包围球的中心。*/
		this.center=null;
		/**包围球的半径。*/
		this.radius=NaN;
		this.center=center;
		this.radius=radius;
	}

	__class(BoundSphere,'laya.d3.math.BoundSphere');
	var __proto=BoundSphere.prototype;
	Laya.imps(__proto,{"laya.d3.core.IClone":true})
	__proto.toDefault=function(){
		this.center.toDefault();
		this.radius=0;
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		var dest=destObject;
		this.center.cloneTo(dest.center);
		dest.radius=this.radius;
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var dest=/*__JS__ */new this.constructor(new Vector3(),new Vector3());
		this.cloneTo(dest);
		return dest;
	}

	BoundSphere.createFromSubPoints=function(points,start,count,out){
		if (points==null){
			throw new Error("points");
		}
		if (start < 0 || start >=points.length){
			throw new Error("start"+start+"Must be in the range [0, "+(points.length-1)+"]");
		}
		if (count < 0 || (start+count)> points.length){
			throw new Error("count"+count+"Must be in the range <= "+points.length+"}");
		};
		var upperEnd=start+count;
		var center=BoundSphere._tempVector3;
		center.elements[0]=0;
		center.elements[1]=0;
		center.elements[2]=0;
		for (var i=start;i < upperEnd;++i){
			Vector3.add(points[i],center,center);
		};
		var outCenter=out.center;
		Vector3.scale(center,1 / count,outCenter);
		var radius=0.0;
		for (i=start;i < upperEnd;++i){
			var distance=Vector3.distanceSquared(outCenter,points[i]);
			if (distance > radius)
				radius=distance;
		}
		out.radius=Math.sqrt(radius);
	}

	BoundSphere.createfromPoints=function(points,out){
		if (points==null){
			throw new Error("points");
		}
		BoundSphere.createFromSubPoints(points,0,points.length,out);
	}

	__static(BoundSphere,
	['_tempVector3',function(){return this._tempVector3=new Vector3();}
	]);
	return BoundSphere;
})()


/**
*<code>Collision</code> 类用于检测碰撞。
*/
//class laya.d3.math.Collision
var Collision=(function(){
	/**
	*创建一个 <code>Collision</code> 实例。
	*/
	function Collision(){}
	__class(Collision,'laya.d3.math.Collision');

	Collision.intersectsPlaneAndPoint=function(plane,point){
		var distance=Vector3.dot(plane.normal,point)+plane.distance;
		if (distance > 0)
			return Plane.PlaneIntersectionType_Front;
		if (distance < 0)
			return Plane.PlaneIntersectionType_Back;
		return Plane.PlaneIntersectionType_Intersecting;
	}

	Collision.intersectsPlaneAndSphere=function(plane,sphere){
		var sphereR=sphere.radius;
		var distance=Vector3.dot(plane.normal,sphere.center)+plane.distance;
		if (distance > sphereR)
			return Plane.PlaneIntersectionType_Front;
		if (distance <-sphereR)
			return Plane.PlaneIntersectionType_Back;
		return Plane.PlaneIntersectionType_Intersecting;
	}

	__static(Collision,
	['_tempV30',function(){return this._tempV30=new Vector3();},'_tempV31',function(){return this._tempV31=new Vector3();},'_tempV32',function(){return this._tempV32=new Vector3();},'_tempV33',function(){return this._tempV33=new Vector3();},'_tempV34',function(){return this._tempV34=new Vector3();},'_tempV35',function(){return this._tempV35=new Vector3();},'_tempV36',function(){return this._tempV36=new Vector3();}
	]);
	return Collision;
})()


/**
*<code>ContainmentType</code> 类用于定义空间物体位置关系。
*/
//class laya.d3.math.ContainmentType
var ContainmentType=(function(){
	function ContainmentType(){}
	__class(ContainmentType,'laya.d3.math.ContainmentType');
	ContainmentType.Disjoint=0;
	ContainmentType.Contains=1;
	ContainmentType.Intersects=2;
	return ContainmentType;
})()


/**
*<code>MathUtils</code> 类用于创建数学工具。
*/
//class laya.d3.math.MathUtils3D
var MathUtils3D=(function(){
	/**
	*创建一个 <code>MathUtils</code> 实例。
	*/
	function MathUtils3D(){}
	__class(MathUtils3D,'laya.d3.math.MathUtils3D');
	MathUtils3D.isZero=function(v){
		return Math.abs(v)< MathUtils3D.zeroTolerance;
	}

	MathUtils3D.nearEqual=function(n1,n2){
		if (MathUtils3D.isZero(n1-n2))
			return true;
		return false;
	}

	MathUtils3D.fastInvSqrt=function(value){
		if (MathUtils3D.isZero(value))
			return value;
		return 1.0 / Math.sqrt(value);
	}

	__static(MathUtils3D,
	['zeroTolerance',function(){return this.zeroTolerance=1e-6;},'MaxValue',function(){return this.MaxValue=3.40282347e+38;},'MinValue',function(){return this.MinValue=-3.40282347e+38;}
	]);
	return MathUtils3D;
})()


/**
*<code>Matrix3x3</code> 类用于创建3x3矩阵。
*/
//class laya.d3.math.Matrix3x3
var Matrix3x3=(function(){
	function Matrix3x3(){
		/**矩阵元素数组*/
		//this.elements=null;
		var e=this.elements=new Float32Array(9);
		e[0]=1;
		e[1]=0;
		e[2]=0;
		e[3]=0;
		e[4]=1;
		e[5]=0;
		e[6]=0;
		e[7]=0;
		e[8]=1;
	}

	__class(Matrix3x3,'laya.d3.math.Matrix3x3');
	var __proto=Matrix3x3.prototype;
	Laya.imps(__proto,{"laya.d3.core.IClone":true})
	/**
	*计算3x3矩阵的行列式
	*@return 矩阵的行列式
	*/
	__proto.determinant=function(){
		var f=this.elements;
		var a00=f[0],a01=f[1],a02=f[2];
		var a10=f[3],a11=f[4],a12=f[5];
		var a20=f[6],a21=f[7],a22=f[8];
		return a00 *(a22 *a11-a12 *a21)+a01 *(-a22 *a10+a12 *a20)+a02 *(a21 *a10-a11 *a20);
	}

	/**
	*通过一个二维向量转换3x3矩阵
	*@param tra 转换向量
	*@param out 输出矩阵
	*/
	__proto.translate=function(trans,out){
		var e=out.elements;
		var f=this.elements;
		var g=trans.elements;
		var a00=f[0],a01=f[1],a02=f[2];
		var a10=f[3],a11=f[4],a12=f[5];
		var a20=f[6],a21=f[7],a22=f[8];
		var x=g[0],y=g[1];
		e[0]=a00;
		e[1]=a01;
		e[2]=a02;
		e[3]=a10;
		e[4]=a11;
		e[5]=a12;
		e[6]=x *a00+y *a10+a20;
		e[7]=x *a01+y *a11+a21;
		e[8]=x *a02+y *a12+a22;
	}

	/**
	*根据指定角度旋转3x3矩阵
	*@param rad 旋转角度
	*@param out 输出矩阵
	*/
	__proto.rotate=function(rad,out){
		var e=out.elements;
		var f=this.elements;
		var a00=f[0],a01=f[1],a02=f[2];
		var a10=f[3],a11=f[4],a12=f[5];
		var a20=f[6],a21=f[7],a22=f[8];
		var s=Math.sin(rad);
		var c=Math.cos(rad);
		e[0]=c *a00+s *a10;
		e[1]=c *a01+s *a11;
		e[2]=c *a02+s *a12;
		e[3]=c *a10-s *a00;
		e[4]=c *a11-s *a01;
		e[5]=c *a12-s *a02;
		e[6]=a20;
		e[7]=a21;
		e[8]=a22;
	}

	/**
	*根据制定缩放3x3矩阵
	*@param scale 缩放值
	*@param out 输出矩阵
	*/
	__proto.scale=function(scale,out){
		var e=out.elements;
		var f=this.elements;
		var g=scale.elements;
		var x=g[0],y=g[1];
		e[0]=x *f[0];
		e[1]=x *f[1];
		e[2]=x *f[2];
		e[3]=y *f[3];
		e[4]=y *f[4];
		e[5]=y *f[5];
		e[6]=f[6];
		e[7]=f[7];
		e[8]=f[8];
	}

	/**
	*计算3x3矩阵的逆矩阵
	*@param out 输出的逆矩阵
	*/
	__proto.invert=function(out){
		var e=out.elements;
		var f=this.elements;
		var a00=f[0],a01=f[1],a02=f[2];
		var a10=f[3],a11=f[4],a12=f[5];
		var a20=f[6],a21=f[7],a22=f[8];
		var b01=a22 *a11-a12 *a21;
		var b11=-a22 *a10+a12 *a20;
		var b21=a21 *a10-a11 *a20;
		var det=a00 *b01+a01 *b11+a02 *b21;
		if (!det){
			out=null;
		}
		det=1.0 / det;
		e[0]=b01 *det;
		e[1]=(-a22 *a01+a02 *a21)*det;
		e[2]=(a12 *a01-a02 *a11)*det;
		e[3]=b11 *det;
		e[4]=(a22 *a00-a02 *a20)*det;
		e[5]=(-a12 *a00+a02 *a10)*det;
		e[6]=b21 *det;
		e[7]=(-a21 *a00+a01 *a20)*det;
		e[8]=(a11 *a00-a01 *a10)*det;
	}

	/**
	*计算3x3矩阵的转置矩阵
	*@param out 输出矩阵
	*/
	__proto.transpose=function(out){
		var e=out.elements;
		var f=this.elements;
		if (out===this){
			var a01=f[1],a02=f[2],a12=f[5];
			e[1]=f[3];
			e[2]=f[6];
			e[3]=a01;
			e[5]=f[7];
			e[6]=a02;
			e[7]=a12;
			}else {
			e[0]=f[0];
			e[1]=f[3];
			e[2]=f[6];
			e[3]=f[1];
			e[4]=f[4];
			e[5]=f[7];
			e[6]=f[2];
			e[7]=f[5];
			e[8]=f[8];
		}
	}

	/**设置已有的矩阵为单位矩阵*/
	__proto.identity=function(){
		var e=this.elements;
		e[0]=1;
		e[1]=0;
		e[2]=0;
		e[3]=0;
		e[4]=1;
		e[5]=0;
		e[6]=0;
		e[7]=0;
		e[8]=1;
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		var i,s,d;
		s=this.elements;
		d=destObject.elements;
		if (s===d){
			return;
		}
		for (i=0;i < 9;++i){
			d[i]=s[i];
		}
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var dest=/*__JS__ */new this.constructor();
		this.cloneTo(dest);
		return dest;
	}

	Matrix3x3.createFromTranslation=function(trans,out){
		var e=out.elements;
		var g=trans.elements;
		out[0]=1;
		out[1]=0;
		out[2]=0;
		out[3]=0;
		out[4]=1;
		out[5]=0;
		out[6]=g[0];
		out[7]=g[1];
		out[8]=1;
	}

	Matrix3x3.createFromRotation=function(rad,out){
		var e=out.elements;
		var s=Math.sin(rad),c=Math.cos(rad);
		e[0]=c;
		e[1]=s;
		e[2]=0;
		e[3]=-s;
		e[4]=c;
		e[5]=0;
		e[6]=0;
		e[7]=0;
		e[8]=1;
	}

	Matrix3x3.createFromScaling=function(scale,out){
		var e=out.elements;
		var g=scale.elements;
		e[0]=g[0];
		e[1]=0;
		e[2]=0;
		e[3]=0;
		e[4]=g[1];
		e[5]=0;
		e[6]=0;
		e[7]=0;
		e[8]=1;
	}

	Matrix3x3.createFromMatrix4x4=function(sou,out){
		out[0]=sou[0];
		out[1]=sou[1];
		out[2]=sou[2];
		out[3]=sou[4];
		out[4]=sou[5];
		out[5]=sou[6];
		out[6]=sou[8];
		out[7]=sou[9];
		out[8]=sou[10];
	}

	Matrix3x3.multiply=function(left,right,out){
		var e=out.elements;
		var f=left.elements;
		var g=right.elements;
		var a00=f[0],a01=f[1],a02=f[2];
		var a10=f[3],a11=f[4],a12=f[5];
		var a20=f[6],a21=f[7],a22=f[8];
		var b00=g[0],b01=g[1],b02=g[2];
		var b10=g[3],b11=g[4],b12=g[5];
		var b20=g[6],b21=g[7],b22=g[8];
		e[0]=b00 *a00+b01 *a10+b02 *a20;
		e[1]=b00 *a01+b01 *a11+b02 *a21;
		e[2]=b00 *a02+b01 *a12+b02 *a22;
		e[3]=b10 *a00+b11 *a10+b12 *a20;
		e[4]=b10 *a01+b11 *a11+b12 *a21;
		e[5]=b10 *a02+b11 *a12+b12 *a22;
		e[6]=b20 *a00+b21 *a10+b22 *a20;
		e[7]=b20 *a01+b21 *a11+b22 *a21;
		e[8]=b20 *a02+b21 *a12+b22 *a22;
	}

	Matrix3x3.lookAt=function(eye,target,up,out){
		Vector3.subtract(eye,target,Matrix3x3._tempV30);
		Vector3.normalize(Matrix3x3._tempV30,Matrix3x3._tempV30);
		Vector3.cross(up,Matrix3x3._tempV30,Matrix3x3._tempV31);
		Vector3.normalize(Matrix3x3._tempV31,Matrix3x3._tempV31);
		Vector3.cross(Matrix3x3._tempV30,Matrix3x3._tempV31,Matrix3x3._tempV32);
		var v0e=Matrix3x3._tempV30.elements;
		var v1e=Matrix3x3._tempV31.elements;
		var v2e=Matrix3x3._tempV32.elements;
		var me=out.elements;
		me[0]=v1e[0];
		me[3]=v1e[1];
		me[6]=v1e[2];
		me[1]=v2e[0];
		me[4]=v2e[1];
		me[7]=v2e[2];
		me[2]=v0e[0];
		me[5]=v0e[1];
		me[8]=v0e[2];
	}

	Matrix3x3.DEFAULT=new Matrix3x3();
	__static(Matrix3x3,
	['_tempV30',function(){return this._tempV30=new Vector3();},'_tempV31',function(){return this._tempV31=new Vector3();},'_tempV32',function(){return this._tempV32=new Vector3();}
	]);
	return Matrix3x3;
})()


/**
*<code>Matrix4x4</code> 类用于创建4x4矩阵。
*/
//class laya.d3.math.Matrix4x4
var Matrix4x4=(function(){
	function Matrix4x4(m11,m12,m13,m14,m21,m22,m23,m24,m31,m32,m33,m34,m41,m42,m43,m44){
		/**矩阵元素数组*/
		//this.elements=null;
		(m11===void 0)&& (m11=1);
		(m12===void 0)&& (m12=0);
		(m13===void 0)&& (m13=0);
		(m14===void 0)&& (m14=0);
		(m21===void 0)&& (m21=0);
		(m22===void 0)&& (m22=1);
		(m23===void 0)&& (m23=0);
		(m24===void 0)&& (m24=0);
		(m31===void 0)&& (m31=0);
		(m32===void 0)&& (m32=0);
		(m33===void 0)&& (m33=1);
		(m34===void 0)&& (m34=0);
		(m41===void 0)&& (m41=0);
		(m42===void 0)&& (m42=0);
		(m43===void 0)&& (m43=0);
		(m44===void 0)&& (m44=1);
		var e=this.elements=new Float32Array(16);
		e[0]=m11;
		e[1]=m12;
		e[2]=m13;
		e[3]=m14;
		e[4]=m21;
		e[5]=m22;
		e[6]=m23;
		e[7]=m24;
		e[8]=m31;
		e[9]=m32;
		e[10]=m33;
		e[11]=m34;
		e[12]=m41;
		e[13]=m42;
		e[14]=m43;
		e[15]=m44;
	}

	__class(Matrix4x4,'laya.d3.math.Matrix4x4');
	var __proto=Matrix4x4.prototype;
	Laya.imps(__proto,{"laya.d3.core.IClone":true})
	__proto.getElementByRowColumn=function(row,column){
		if (row < 0 || row > 3)
			throw new Error("row","Rows and columns for matrices run from 0 to 3, inclusive.");
		if (column < 0 || column > 3)
			throw new Error("column","Rows and columns for matrices run from 0 to 3, inclusive.");
		return this.elements[(row *4)+column];
	}

	__proto.setElementByRowColumn=function(row,column,value){
		if (row < 0 || row > 3)
			throw new Error("row","Rows and columns for matrices run from 0 to 3, inclusive.");
		if (column < 0 || column > 3)
			throw new Error("column","Rows and columns for matrices run from 0 to 3, inclusive.");
		this.elements[(row *4)+column]=value;
	}

	/**
	*判断两个4x4矩阵的值是否相等。
	*@param other 4x4矩阵
	*/
	__proto.equalsOtherMatrix=function(other){
		var e=this.elements;
		var oe=other.elements;
		return (MathUtils3D.nearEqual(e[0],oe[0])&& MathUtils3D.nearEqual(e[1],oe[1])&& MathUtils3D.nearEqual(e[2],oe[2])&& MathUtils3D.nearEqual(e[3],oe[3])&& MathUtils3D.nearEqual(e[4],oe[4])&& MathUtils3D.nearEqual(e[5],oe[5])&& MathUtils3D.nearEqual(e[6],oe[6])&& MathUtils3D.nearEqual(e[7],oe[7])&& MathUtils3D.nearEqual(e[8],oe[8])&& MathUtils3D.nearEqual(e[9],oe[9])&& MathUtils3D.nearEqual(e[10],oe[10])&& MathUtils3D.nearEqual(e[11],oe[11])&& MathUtils3D.nearEqual(e[12],oe[12])&& MathUtils3D.nearEqual(e[13],oe[13])&& MathUtils3D.nearEqual(e[14],oe[14])&& MathUtils3D.nearEqual(e[15],oe[15]));
	}

	/**
	*分解矩阵为平移向量、旋转四元数、缩放向量。
	*@param translation 平移向量。
	*@param rotation 旋转四元数。
	*@param scale 缩放向量。
	*@return 是否分解成功。
	*/
	__proto.decomposeTransRotScale=function(translation,rotation,scale){
		var rotationMatrix=Matrix4x4._tempMatrix4x4;
		if (this.decomposeTransRotMatScale(translation,rotationMatrix,scale)){
			Quaternion.createFromMatrix4x4(rotationMatrix,rotation);
			return true;
			}else {
			rotation.identity();
			return false;
		}
	}

	/**
	*分解矩阵为平移向量、旋转矩阵、缩放向量。
	*@param translation 平移向量。
	*@param rotationMatrix 旋转矩阵。
	*@param scale 缩放向量。
	*@return 是否分解成功。
	*/
	__proto.decomposeTransRotMatScale=function(translation,rotationMatrix,scale){
		var e=this.elements;
		var te=translation.elements;
		var re=rotationMatrix.elements;
		var se=scale.elements;
		te[0]=e[12];
		te[1]=e[13];
		te[2]=e[14];
		var m11=e[0],m12=e[1],m13=e[2];
		var m21=e[4],m22=e[5],m23=e[6];
		var m31=e[8],m32=e[9],m33=e[10];
		var sX=se[0]=Math.sqrt((m11 *m11)+(m12 *m12)+(m13 *m13));
		var sY=se[1]=Math.sqrt((m21 *m21)+(m22 *m22)+(m23 *m23));
		var sZ=se[2]=Math.sqrt((m31 *m31)+(m32 *m32)+(m33 *m33));
		if (MathUtils3D.isZero(sX)|| MathUtils3D.isZero(sY)|| MathUtils3D.isZero(sZ)){
			re[1]=re[2]=re[3]=re[4]=re[6]=re[7]=re[8]=re[9]=re[11]=re[12]=re[13]=re[14]=0;
			re[0]=re[5]=re[10]=re[15]=1;
			return false;
		};
		var at=Matrix4x4._tempVector0;
		var atE=at.elements;
		atE[0]=m31 / sZ;
		atE[1]=m32 / sZ;
		atE[2]=m33 / sZ;
		var tempRight=Matrix4x4._tempVector1;
		var tempRightE=tempRight.elements;
		tempRightE[0]=m11 / sX;
		tempRightE[1]=m12 / sX;
		tempRightE[2]=m13 / sX;
		var up=Matrix4x4._tempVector2;
		Vector3.cross(at,tempRight,up);
		var right=Matrix4x4._tempVector1;
		Vector3.cross(up,at,right);
		re[3]=re[7]=re[11]=re[12]=re[13]=re[14]=0;
		re[15]=1;
		re[0]=right.x;
		re[1]=right.y;
		re[2]=right.z;
		re[4]=up.x;
		re[5]=up.y;
		re[6]=up.z;
		re[8]=at.x;
		re[9]=at.y;
		re[10]=at.z;
		((re[0] *m11+re[1] *m12+re[2] *m13)< 0.0)&& (se[0]=-sX);
		((re[4] *m21+re[5] *m22+re[6] *m23)< 0.0)&& (se[1]=-sY);
		((re[8] *m31+re[9] *m32+re[10] *m33)< 0.0)&& (se[2]=-sZ);
		return true;
	}

	/**
	*分解旋转矩阵的旋转为YawPitchRoll欧拉角。
	*@param out float yaw
	*@param out float pitch
	*@param out float roll
	*@return
	*/
	__proto.decomposeYawPitchRoll=function(yawPitchRoll){
		var yawPitchRollE=yawPitchRoll.elements;
		var pitch=Math.asin(-this.elements[9]);
		yawPitchRollE[1]=pitch;
		var test=Math.cos(pitch);
		if (test > MathUtils3D.zeroTolerance){
			yawPitchRollE[2]=Math.atan2(this.elements[1],this.elements[5]);
			yawPitchRollE[0]=Math.atan2(this.elements[8],this.elements[10]);
			}else {
			yawPitchRollE[2]=Math.atan2(-this.elements[4],this.elements[0]);
			yawPitchRollE[0]=0.0;
		}
	}

	/**归一化矩阵 */
	__proto.normalize=function(){
		var v=this.elements;
		var c=v[0],d=v[1],e=v[2],g=Math.sqrt(c *c+d *d+e *e);
		if (g){
			if (g==1)
				return;
			}else {
			v[0]=0;
			v[1]=0;
			v[2]=0;
			return;
		}
		g=1 / g;
		v[0]=c *g;
		v[1]=d *g;
		v[2]=e *g;
	}

	/**计算矩阵的转置矩阵*/
	__proto.transpose=function(){
		var e,t;
		e=this.elements;
		t=e[1];
		e[1]=e[4];
		e[4]=t;
		t=e[2];
		e[2]=e[8];
		e[8]=t;
		t=e[3];
		e[3]=e[12];
		e[12]=t;
		t=e[6];
		e[6]=e[9];
		e[9]=t;
		t=e[7];
		e[7]=e[13];
		e[13]=t;
		t=e[11];
		e[11]=e[14];
		e[14]=t;
		return this;
	}

	/**
	*计算一个矩阵的逆矩阵
	*@param out 输出矩阵
	*/
	__proto.invert=function(out){
		var ae=this.elements;
		var oe=out.elements;
		var a00=ae[0],a01=ae[1],a02=ae[2],a03=ae[3],a10=ae[4],a11=ae[5],a12=ae[6],a13=ae[7],a20=ae[8],a21=ae[9],a22=ae[10],a23=ae[11],a30=ae[12],a31=ae[13],a32=ae[14],a33=ae[15],
		b00=a00 *a11-a01 *a10,b01=a00 *a12-a02 *a10,b02=a00 *a13-a03 *a10,b03=a01 *a12-a02 *a11,b04=a01 *a13-a03 *a11,b05=a02 *a13-a03 *a12,b06=a20 *a31-a21 *a30,b07=a20 *a32-a22 *a30,b08=a20 *a33-a23 *a30,b09=a21 *a32-a22 *a31,b10=a21 *a33-a23 *a31,b11=a22 *a33-a23 *a32,
		det=b00 *b11-b01 *b10+b02 *b09+b03 *b08-b04 *b07+b05 *b06;
		if (Math.abs(det)===0.0){
			return;
		}
		det=1.0 / det;
		oe[0]=(a11 *b11-a12 *b10+a13 *b09)*det;
		oe[1]=(a02 *b10-a01 *b11-a03 *b09)*det;
		oe[2]=(a31 *b05-a32 *b04+a33 *b03)*det;
		oe[3]=(a22 *b04-a21 *b05-a23 *b03)*det;
		oe[4]=(a12 *b08-a10 *b11-a13 *b07)*det;
		oe[5]=(a00 *b11-a02 *b08+a03 *b07)*det;
		oe[6]=(a32 *b02-a30 *b05-a33 *b01)*det;
		oe[7]=(a20 *b05-a22 *b02+a23 *b01)*det;
		oe[8]=(a10 *b10-a11 *b08+a13 *b06)*det;
		oe[9]=(a01 *b08-a00 *b10-a03 *b06)*det;
		oe[10]=(a30 *b04-a31 *b02+a33 *b00)*det;
		oe[11]=(a21 *b02-a20 *b04-a23 *b00)*det;
		oe[12]=(a11 *b07-a10 *b09-a12 *b06)*det;
		oe[13]=(a00 *b09-a01 *b07+a02 *b06)*det;
		oe[14]=(a31 *b01-a30 *b03-a32 *b00)*det;
		oe[15]=(a20 *b03-a21 *b01+a22 *b00)*det;
	}

	/**设置矩阵为单位矩阵*/
	__proto.identity=function(){
		var e=this.elements;
		e[1]=e[2]=e[3]=e[4]=e[6]=e[7]=e[8]=e[9]=e[11]=e[12]=e[13]=e[14]=0;
		e[0]=e[5]=e[10]=e[15]=1;
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		var i,s,d;
		s=this.elements;
		d=destObject.elements;
		if (s===d){
			return;
		}
		for (i=0;i < 16;++i){
			d[i]=s[i];
		}
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var dest=/*__JS__ */new this.constructor();
		this.cloneTo(dest);
		return dest;
	}

	/**
	*获取平移向量。
	*@param out 平移向量。
	*/
	__proto.getTranslationVector=function(out){
		var me=this.elements;
		var te=out.elements;
		te[0]=me[12];
		te[1]=me[13];
		te[2]=me[14];
	}

	/**
	*设置平移向量。
	*@param translate 平移向量。
	*/
	__proto.setTranslationVector=function(translate){
		var me=this.elements;
		var ve=translate.elements;
		me[12]=ve[0];
		me[13]=ve[1];
		me[14]=ve[2];
	}

	/**
	*获取前向量。
	*@param out 前向量。
	*/
	__proto.getForward=function(out){
		var me=this.elements;
		var te=out.elements;
		te[0]=-me[8];
		te[1]=-me[9];
		te[2]=-me[10];
	}

	/**
	*设置前向量。
	*@param forward 前向量。
	*/
	__proto.setForward=function(forward){
		var me=this.elements;
		var ve=forward.elements;
		me[8]=-ve[0];
		me[9]=-ve[1];
		me[10]=-ve[2];
	}

	Matrix4x4.createRotationX=function(rad,out){
		var oe=out.elements;
		var s=Math.sin(rad),c=Math.cos(rad);
		oe[1]=oe[2]=oe[3]=oe[4]=oe[7]=oe[8]=oe[11]=oe[12]=oe[13]=oe[14]=0;
		oe[0]=oe[15]=1;
		oe[5]=oe[10]=c;
		oe[6]=s;
		oe[9]=-s;
	}

	Matrix4x4.createRotationY=function(rad,out){
		var oe=out.elements;
		var s=Math.sin(rad),c=Math.cos(rad);
		oe[1]=oe[3]=oe[4]=oe[6]=oe[7]=oe[9]=oe[11]=oe[12]=oe[13]=oe[14]=0;
		oe[5]=oe[15]=1;
		oe[0]=oe[10]=c;
		oe[2]=-s;
		oe[8]=s;
	}

	Matrix4x4.createRotationZ=function(rad,out){
		var oe=out.elements;
		var s=Math.sin(rad),c=Math.cos(rad);
		oe[2]=oe[3]=oe[6]=oe[7]=oe[8]=oe[9]=oe[11]=oe[12]=oe[13]=oe[14]=0;
		oe[10]=oe[15]=1;
		oe[0]=oe[5]=c;
		oe[1]=s;
		oe[4]=-s;
	}

	Matrix4x4.createRotationYawPitchRoll=function(yaw,pitch,roll,result){
		Quaternion.createFromYawPitchRoll(yaw,pitch,roll,Matrix4x4._tempQuaternion);
		Matrix4x4.createRotationQuaternion(Matrix4x4._tempQuaternion,result);
	}

	Matrix4x4.createRotationAxis=function(axis,angle,result){
		var axisE=axis.elements;
		var x=axisE[0];
		var y=axisE[1];
		var z=axisE[2];
		var cos=Math.cos(angle);
		var sin=Math.sin(angle);
		var xx=x *x;
		var yy=y *y;
		var zz=z *z;
		var xy=x *y;
		var xz=x *z;
		var yz=y *z;
		var resultE=result.elements;
		resultE[3]=resultE[7]=resultE[11]=resultE[12]=resultE[13]=resultE[14]=0;
		resultE[15]=1.0;
		resultE[0]=xx+(cos *(1.0-xx));
		resultE[1]=(xy-(cos *xy))+(sin *z);
		resultE[2]=(xz-(cos *xz))-(sin *y);
		resultE[4]=(xy-(cos *xy))-(sin *z);
		resultE[5]=yy+(cos *(1.0-yy));
		resultE[6]=(yz-(cos *yz))+(sin *x);
		resultE[8]=(xz-(cos *xz))+(sin *y);
		resultE[9]=(yz-(cos *yz))-(sin *x);
		resultE[10]=zz+(cos *(1.0-zz));
	}

	Matrix4x4.createRotationQuaternion=function(rotation,result){
		var rotationE=rotation.elements;
		var resultE=result.elements;
		var rotationX=rotationE[0];
		var rotationY=rotationE[1];
		var rotationZ=rotationE[2];
		var rotationW=rotationE[3];
		var xx=rotationX *rotationX;
		var yy=rotationY *rotationY;
		var zz=rotationZ *rotationZ;
		var xy=rotationX *rotationY;
		var zw=rotationZ *rotationW;
		var zx=rotationZ *rotationX;
		var yw=rotationY *rotationW;
		var yz=rotationY *rotationZ;
		var xw=rotationX *rotationW;
		resultE[3]=resultE[7]=resultE[11]=resultE[12]=resultE[13]=resultE[14]=0;
		resultE[15]=1.0;
		resultE[0]=1.0-(2.0 *(yy+zz));
		resultE[1]=2.0 *(xy+zw);
		resultE[2]=2.0 *(zx-yw);
		resultE[4]=2.0 *(xy-zw);
		resultE[5]=1.0-(2.0 *(zz+xx));
		resultE[6]=2.0 *(yz+xw);
		resultE[8]=2.0 *(zx+yw);
		resultE[9]=2.0 *(yz-xw);
		resultE[10]=1.0-(2.0 *(yy+xx));
	}

	Matrix4x4.createTranslate=function(trans,out){
		var te=trans.elements;
		var oe=out.elements;
		oe[4]=oe[8]=oe[1]=oe[9]=oe[2]=oe[6]=oe[3]=oe[7]=oe[11]=0;
		oe[0]=oe[5]=oe[10]=oe[15]=1;
		oe[12]=te[0];
		oe[13]=te[1];
		oe[14]=te[2];
	}

	Matrix4x4.createScaling=function(scale,out){
		var se=scale.elements;
		var oe=out.elements;
		oe[0]=se[0];
		oe[5]=se[1];
		oe[10]=se[2];
		oe[1]=oe[4]=oe[8]=oe[12]=oe[9]=oe[13]=oe[2]=oe[6]=oe[14]=oe[3]=oe[7]=oe[11]=0;
		oe[15]=1;
	}

	Matrix4x4.multiply=function(left,right,out){
		var i,e,a,b,ai0,ai1,ai2,ai3;
		e=out.elements;
		a=left.elements;
		b=right.elements;
		if (e===b){
			b=new Float32Array(16);
			for (i=0;i < 16;++i){
				b[i]=e[i];
			}
		}
		for (i=0;i < 4;i++){
			ai0=a[i];
			ai1=a[i+4];
			ai2=a[i+8];
			ai3=a[i+12];
			e[i]=ai0 *b[0]+ai1 *b[1]+ai2 *b[2]+ai3 *b[3];
			e[i+4]=ai0 *b[4]+ai1 *b[5]+ai2 *b[6]+ai3 *b[7];
			e[i+8]=ai0 *b[8]+ai1 *b[9]+ai2 *b[10]+ai3 *b[11];
			e[i+12]=ai0 *b[12]+ai1 *b[13]+ai2 *b[14]+ai3 *b[15];
		}
	}

	Matrix4x4.createFromQuaternion=function(rotation,out){
		var e=out.elements;
		var q=rotation.elements;
		var x=q[0],y=q[1],z=q[2],w=q[3];
		var x2=x+x;
		var y2=y+y;
		var z2=z+z;
		var xx=x *x2;
		var yx=y *x2;
		var yy=y *y2;
		var zx=z *x2;
		var zy=z *y2;
		var zz=z *z2;
		var wx=w *x2;
		var wy=w *y2;
		var wz=w *z2;
		e[0]=1-yy-zz;
		e[1]=yx+wz;
		e[2]=zx-wy;
		e[3]=0;
		e[4]=yx-wz;
		e[5]=1-xx-zz;
		e[6]=zy+wx;
		e[7]=0;
		e[8]=zx+wy;
		e[9]=zy-wx;
		e[10]=1-xx-yy;
		e[11]=0;
		e[12]=0;
		e[13]=0;
		e[14]=0;
		e[15]=1;
	}

	Matrix4x4.createAffineTransformation=function(trans,rot,scale,out){
		var te=trans.elements;
		var re=rot.elements;
		var se=scale.elements;
		var oe=out.elements;
		var x=re[0],y=re[1],z=re[2],w=re[3],x2=x+x,y2=y+y,z2=z+z;
		var xx=x *x2,xy=x *y2,xz=x *z2,yy=y *y2,yz=y *z2,zz=z *z2;
		var wx=w *x2,wy=w *y2,wz=w *z2,sx=se[0],sy=se[1],sz=se[2];
		oe[0]=(1-(yy+zz))*sx;
		oe[1]=(xy+wz)*sx;
		oe[2]=(xz-wy)*sx;
		oe[3]=0;
		oe[4]=(xy-wz)*sy;
		oe[5]=(1-(xx+zz))*sy;
		oe[6]=(yz+wx)*sy;
		oe[7]=0;
		oe[8]=(xz+wy)*sz;
		oe[9]=(yz-wx)*sz;
		oe[10]=(1-(xx+yy))*sz;
		oe[11]=0;
		oe[12]=te[0];
		oe[13]=te[1];
		oe[14]=te[2];
		oe[15]=1;
	}

	Matrix4x4.createLookAt=function(eye,target,up,out){
		var oE=out.elements;
		var xaxis=Matrix4x4._tempVector0;
		var yaxis=Matrix4x4._tempVector1;
		var zaxis=Matrix4x4._tempVector2;
		Vector3.subtract(eye,target,zaxis);
		Vector3.normalize(zaxis,zaxis);
		Vector3.cross(up,zaxis,xaxis);
		Vector3.normalize(xaxis,xaxis);
		Vector3.cross(zaxis,xaxis,yaxis);
		out.identity();
		oE[0]=xaxis.x;
		oE[4]=xaxis.y;
		oE[8]=xaxis.z;
		oE[1]=yaxis.x;
		oE[5]=yaxis.y;
		oE[9]=yaxis.z;
		oE[2]=zaxis.x;
		oE[6]=zaxis.y;
		oE[10]=zaxis.z;
		oE[12]=-Vector3.dot(xaxis,eye);
		oE[13]=-Vector3.dot(yaxis,eye);
		oE[14]=-Vector3.dot(zaxis,eye);
	}

	Matrix4x4.createPerspective=function(fov,aspect,near,far,out){
		var oe=out.elements;
		var f=1.0 / Math.tan(fov / 2),nf=1 / (near-far);
		oe[0]=f / aspect;
		oe[5]=f;
		oe[10]=(far+near)*nf;
		oe[11]=-1;
		oe[14]=(2 *far *near)*nf;
		oe[1]=oe[2]=oe[3]=oe[4]=oe[6]=oe[7]=oe[8]=oe[9]=oe[12]=oe[13]=oe[15]=0;
	}

	Matrix4x4.createOrthoOffCenterRH=function(left,right,bottom,top,near,far,out){
		var oe=out.elements;
		var lr=1 / (left-right);
		var bt=1 / (bottom-top);
		var nf=1 / (near-far);
		oe[1]=oe[2]=oe[3]=oe[4]=oe[6]=oe[7]=oe[8]=oe[9]=oe[11]=0;
		oe[15]=1;
		oe[0]=-2 *lr;
		oe[5]=-2 *bt;
		oe[10]=2 *nf;
		oe[12]=(left+right)*lr;
		oe[13]=(top+bottom)*bt;
		oe[14]=(far+near)*nf;
	}

	Matrix4x4.translation=function(v3,out){
		var ve=v3.elements;
		var oe=out.elements;
		oe[0]=oe[5]=oe[10]=oe[15]=1;
		oe[12]=ve[0];
		oe[13]=ve[1];
		oe[14]=ve[2];
	}

	__static(Matrix4x4,
	['_tempMatrix4x4',function(){return this._tempMatrix4x4=new Matrix4x4();},'_tempVector0',function(){return this._tempVector0=new Vector3();},'_tempVector1',function(){return this._tempVector1=new Vector3();},'_tempVector2',function(){return this._tempVector2=new Vector3();},'_tempQuaternion',function(){return this._tempQuaternion=new Quaternion();},'DEFAULT',function(){return this.DEFAULT=new Matrix4x4();},'ZERO',function(){return this.ZERO=new Matrix4x4(
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0);}
	]);
	return Matrix4x4;
})()


/**
*<code>Plane</code> 类用于创建平面。
*/
//class laya.d3.math.Plane
var Plane=(function(){
	function Plane(normal,d){
		/**平面的向量*/
		this.normal=null;
		/**平面到坐标系原点的距离*/
		this.distance=NaN;
		(d===void 0)&& (d=0);
		this.normal=normal;
		this.distance=d;
	}

	__class(Plane,'laya.d3.math.Plane');
	var __proto=Plane.prototype;
	/**
	*更改平面法线向量的系数，使之成单位长度。
	*/
	__proto.normalize=function(){
		var normalE=this.normal.elements;
		var normalEX=normalE[0];
		var normalEY=normalE[1];
		var normalEZ=normalE[2];
		var magnitude=1 / Math.sqrt(normalEX *normalEX+normalEY *normalEY+normalEZ *normalEZ);
		normalE[0]=normalEX *magnitude;
		normalE[1]=normalEY *magnitude;
		normalE[2]=normalEZ *magnitude;
		this.distance *=magnitude;
	}

	Plane.createPlaneBy3P=function(point1,point2,point3){
		var point1e=point1.elements;
		var point2e=point2.elements;
		var point3e=point3.elements;
		var x1=point2e[0]-point1e[0];
		var y1=point2e[1]-point1e[1];
		var z1=point2e[2]-point1e[2];
		var x2=point3e[0]-point1e[0];
		var y2=point3e[1]-point1e[1];
		var z2=point3e[2]-point1e[2];
		var yz=(y1 *z2)-(z1 *y2);
		var xz=(z1 *x2)-(x1 *z2);
		var xy=(x1 *y2)-(y1 *x2);
		var invPyth=1 / (Math.sqrt((yz *yz)+(xz *xz)+(xy *xy)));
		var x=yz *invPyth;
		var y=xz *invPyth;
		var z=xy *invPyth;
		var TEMPVec3e=Plane._TEMPVec3.elements;
		TEMPVec3e[0]=x;
		TEMPVec3e[1]=y;
		TEMPVec3e[2]=z;
		var d=-((x *point1e[0])+(y *point1e[1])+(z *point1e[2]));
		var plane=new Plane(Plane._TEMPVec3,d);
		return plane;
	}

	Plane.PlaneIntersectionType_Back=0;
	Plane.PlaneIntersectionType_Front=1;
	Plane.PlaneIntersectionType_Intersecting=2;
	__static(Plane,
	['_TEMPVec3',function(){return this._TEMPVec3=new Vector3();}
	]);
	return Plane;
})()


/**
*<code>Quaternion</code> 类用于创建四元数。
*/
//class laya.d3.math.Quaternion
var Quaternion=(function(){
	function Quaternion(x,y,z,w){
		this.elements=new Float32Array(4);
		(x===void 0)&& (x=0);
		(y===void 0)&& (y=0);
		(z===void 0)&& (z=0);
		(w===void 0)&& (w=1);
		this.elements[0]=x;
		this.elements[1]=y;
		this.elements[2]=z;
		this.elements[3]=w;
	}

	__class(Quaternion,'laya.d3.math.Quaternion');
	var __proto=Quaternion.prototype;
	Laya.imps(__proto,{"laya.d3.core.IClone":true})
	/**
	*根据缩放值缩放四元数
	*@param scale 缩放值
	*@param out 输出四元数
	*/
	__proto.scaling=function(scaling,out){
		var e=out.elements;
		var f=this.elements;
		e[0]=f[0] *scaling;
		e[1]=f[1] *scaling;
		e[2]=f[2] *scaling;
		e[3]=f[3] *scaling;
	}

	/**
	*归一化四元数
	*@param out 输出四元数
	*/
	__proto.normalize=function(out){
		var e=out.elements;
		var f=this.elements;
		var x=f[0],y=f[1],z=f[2],w=f[3];
		var len=x *x+y *y+z *z+w *w;
		if (len > 0){
			len=1 / Math.sqrt(len);
			e[0]=x *len;
			e[1]=y *len;
			e[2]=z *len;
			e[3]=w *len;
		}
	}

	/**
	*计算四元数的长度
	*@return 长度
	*/
	__proto.length=function(){
		var f=this.elements;
		var x=f[0],y=f[1],z=f[2],w=f[3];
		return Math.sqrt(x *x+y *y+z *z+w *w);
	}

	/**
	*根据绕X轴的角度旋转四元数
	*@param rad 角度
	*@param out 输出四元数
	*/
	__proto.rotateX=function(rad,out){
		var e=out.elements;
		var f=this.elements;
		rad *=0.5;
		var ax=f[0],ay=f[1],az=f[2],aw=f[3];
		var bx=Math.sin(rad),bw=Math.cos(rad);
		e[0]=ax *bw+aw *bx;
		e[1]=ay *bw+az *bx;
		e[2]=az *bw-ay *bx;
		e[3]=aw *bw-ax *bx;
	}

	/**
	*根据绕Y轴的制定角度旋转四元数
	*@param rad 角度
	*@param out 输出四元数
	*/
	__proto.rotateY=function(rad,out){
		var e=out.elements;
		var f=this.elements;
		rad *=0.5;
		var ax=f[0],ay=f[1],az=f[2],aw=f[3],by=Math.sin(rad),bw=Math.cos(rad);
		e[0]=ax *bw-az *by;
		e[1]=ay *bw+aw *by;
		e[2]=az *bw+ax *by;
		e[3]=aw *bw-ay *by;
	}

	/**
	*根据绕Z轴的制定角度旋转四元数
	*@param rad 角度
	*@param out 输出四元数
	*/
	__proto.rotateZ=function(rad,out){
		var e=out.elements;
		var f=this.elements;
		rad *=0.5;
		var ax=f[0],ay=f[1],az=f[2],aw=f[3],bz=Math.sin(rad),bw=Math.cos(rad);
		e[0]=ax *bw+ay *bz;
		e[1]=ay *bw-ax *bz;
		e[2]=az *bw+aw *bz;
		e[3]=aw *bw-az *bz;
	}

	/**
	*分解四元数到欧拉角（顺序为Yaw、Pitch、Roll），参考自http://xboxforums.create.msdn.com/forums/p/4574/23988.aspx#23988,问题绕X轴翻转超过±90度时有，会产生瞬间反转
	*@param quaternion 源四元数
	*@param out 欧拉角值
	*/
	__proto.getYawPitchRoll=function(out){
		Vector3.transformQuat(Vector3.ForwardRH,this,Quaternion.TEMPVector31);
		Vector3.transformQuat(Vector3.Up,this,Quaternion.TEMPVector32);
		var upe=Quaternion.TEMPVector32.elements;
		Quaternion.angleTo(Vector3.ZERO,Quaternion.TEMPVector31,Quaternion.TEMPVector33);
		var anglee=Quaternion.TEMPVector33.elements;
		if (anglee[0]==Math.PI / 2){
			anglee[1]=Quaternion.arcTanAngle(upe[2],upe[0]);
			anglee[2]=0;
			}else if (anglee[0]==-Math.PI / 2){
			anglee[1]=Quaternion.arcTanAngle(-upe[2],-upe[0]);
			anglee[2]=0;
			}else {
			Matrix4x4.createRotationY(-anglee[1],Quaternion.TEMPMatrix0);
			Matrix4x4.createRotationX(-anglee[0],Quaternion.TEMPMatrix1);
			Vector3.transformCoordinate(Quaternion.TEMPVector32,Quaternion.TEMPMatrix0,Quaternion.TEMPVector32);
			Vector3.transformCoordinate(Quaternion.TEMPVector32,Quaternion.TEMPMatrix1,Quaternion.TEMPVector32);
			anglee[2]=Quaternion.arcTanAngle(upe[1],-upe[0]);
		}
		if (anglee[1] <=-Math.PI)
			anglee[1]=Math.PI;
		if (anglee[2] <=-Math.PI)
			anglee[2]=Math.PI;
		if (anglee[1] >=Math.PI && anglee[2] >=Math.PI){
			anglee[1]=0;
			anglee[2]=0;
			anglee[0]=Math.PI-anglee[0];
		};
		var oe=out.elements;
		oe[0]=anglee[1];
		oe[1]=anglee[0];
		oe[2]=anglee[2];
	}

	/**
	*求四元数的逆
	*@param out 输出四元数
	*/
	__proto.invert=function(out){
		var e=out.elements;
		var f=this.elements;
		var a0=f[0],a1=f[1],a2=f[2],a3=f[3];
		var dot=a0 *a0+a1 *a1+a2 *a2+a3 *a3;
		var invDot=dot ? 1.0 / dot :0;
		e[0]=-a0 *invDot;
		e[1]=-a1 *invDot;
		e[2]=-a2 *invDot;
		e[3]=a3 *invDot;
	}

	/**
	*设置四元数为单位算数
	*@param out 输出四元数
	*/
	__proto.identity=function(){
		var e=this.elements;
		e[0]=0;
		e[1]=0;
		e[2]=0;
		e[3]=1;
	}

	/**
	*从Array数组拷贝值。
	*@param array 数组。
	*@param offset 数组偏移。
	*/
	__proto.fromArray=function(array,offset){
		(offset===void 0)&& (offset=0);
		this.elements[0]=array[offset+0];
		this.elements[1]=array[offset+1];
		this.elements[2]=array[offset+2];
		this.elements[3]=array[offset+3];
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		var i,s,d;
		s=this.elements;
		d=destObject.elements;
		if (s===d){
			return;
		}
		for (i=0;i < 4;++i){
			d[i]=s[i];
		}
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var dest=/*__JS__ */new this.constructor();
		this.cloneTo(dest);
		return dest;
	}

	__proto.equals=function(b){
		var ae=this.elements;
		var be=b.elements;
		return MathUtils3D.nearEqual(ae[0],be[0])&& MathUtils3D.nearEqual(ae[1],be[1])&& MathUtils3D.nearEqual(ae[2],be[2])&& MathUtils3D.nearEqual(ae[3],be[3]);
	}

	/**
	*计算长度的平方。
	*@return 长度的平方。
	*/
	__proto.lengthSquared=function(){
		var x=this.elements[0];
		var y=this.elements[1];
		var z=this.elements[2];
		var w=this.elements[3];
		return (x *x)+(y *y)+(z *z)+(w *w);
	}

	/**
	*获取四元数的x值
	*/
	__getset(0,__proto,'x',function(){
		return this.elements[0];
	});

	/**
	*获取四元数的y值
	*/
	__getset(0,__proto,'y',function(){
		return this.elements[1];
	});

	/**
	*获取四元数的z值
	*/
	__getset(0,__proto,'z',function(){
		return this.elements[2];
	});

	/**
	*获取四元数的w值
	*/
	__getset(0,__proto,'w',function(){
		return this.elements[3];
	});

	Quaternion.createFromYawPitchRoll=function(yaw,pitch,roll,out){
		var halfRoll=roll *0.5;
		var halfPitch=pitch *0.5;
		var halfYaw=yaw *0.5;
		var sinRoll=Math.sin(halfRoll);
		var cosRoll=Math.cos(halfRoll);
		var sinPitch=Math.sin(halfPitch);
		var cosPitch=Math.cos(halfPitch);
		var sinYaw=Math.sin(halfYaw);
		var cosYaw=Math.cos(halfYaw);
		var oe=out.elements;
		oe[0]=(cosYaw *sinPitch *cosRoll)+(sinYaw *cosPitch *sinRoll);
		oe[1]=(sinYaw *cosPitch *cosRoll)-(cosYaw *sinPitch *sinRoll);
		oe[2]=(cosYaw *cosPitch *sinRoll)-(sinYaw *sinPitch *cosRoll);
		oe[3]=(cosYaw *cosPitch *cosRoll)+(sinYaw *sinPitch *sinRoll);
	}

	Quaternion.multiply=function(left,right,out){
		var le=left.elements;
		var re=right.elements;
		var oe=out.elements;
		var lx=le[0];
		var ly=le[1];
		var lz=le[2];
		var lw=le[3];
		var rx=re[0];
		var ry=re[1];
		var rz=re[2];
		var rw=re[3];
		var a=(ly *rz-lz *ry);
		var b=(lz *rx-lx *rz);
		var c=(lx *ry-ly *rx);
		var d=(lx *rx+ly *ry+lz *rz);
		oe[0]=(lx *rw+rx *lw)+a;
		oe[1]=(ly *rw+ry *lw)+b;
		oe[2]=(lz *rw+rz *lw)+c;
		oe[3]=lw *rw-d;
	}

	Quaternion.arcTanAngle=function(x,y){
		if (x==0){
			if (y==1)
				return Math.PI / 2;
			return-Math.PI / 2;
		}
		if (x > 0)
			return Math.atan(y / x);
		if (x < 0){
			if (y > 0)
				return Math.atan(y / x)+Math.PI;
			return Math.atan(y / x)-Math.PI;
		}
		return 0;
	}

	Quaternion.angleTo=function(from,location,angle){
		Vector3.subtract(location,from,Quaternion.TEMPVector30);
		Vector3.normalize(Quaternion.TEMPVector30,Quaternion.TEMPVector30);
		angle.elements[0]=Math.asin(Quaternion.TEMPVector30.y);
		angle.elements[1]=Quaternion.arcTanAngle(-Quaternion.TEMPVector30.z,-Quaternion.TEMPVector30.x);
	}

	Quaternion.createFromAxisAngle=function(axis,rad,out){
		var e=out.elements;
		var f=axis.elements;
		rad=rad *0.5;
		var s=Math.sin(rad);
		e[0]=s *f[0];
		e[1]=s *f[1];
		e[2]=s *f[2];
		e[3]=Math.cos(rad);
	}

	Quaternion.createFromMatrix3x3=function(sou,out){
		var e=out.elements;
		var f=sou.elements;
		var fTrace=f[0]+f[4]+f[8];
		var fRoot;
		if (fTrace > 0.0){
			fRoot=Math.sqrt(fTrace+1.0);
			e[3]=0.5 *fRoot;
			fRoot=0.5 / fRoot;
			e[0]=(f[5]-f[7])*fRoot;
			e[1]=(f[6]-f[2])*fRoot;
			e[2]=(f[1]-f[3])*fRoot;
			}else {
			var i=0;
			if (f[4] > f[0])
				i=1;
			if (f[8] > f[i *3+i])
				i=2;
			var j=(i+1)% 3;
			var k=(i+2)% 3;
			fRoot=Math.sqrt(f[i *3+i]-f[j *3+j]-f[k *3+k]+1.0);
			e[i]=0.5 *fRoot;
			fRoot=0.5 / fRoot;
			e[3]=(f[j *3+k]-f[k *3+j])*fRoot;
			e[j]=(f[j *3+i]+f[i *3+j])*fRoot;
			e[k]=(f[k *3+i]+f[i *3+k])*fRoot;
		}
		return;
	}

	Quaternion.createFromMatrix4x4=function(mat,out){
		var me=mat.elements;
		var oe=out.elements;
		var sqrt;
		var half;
		var scale=me[0]+me[5]+me[10];
		if (scale > 0.0){
			sqrt=Math.sqrt(scale+1.0);
			oe[3]=sqrt *0.5;
			sqrt=0.5 / sqrt;
			oe[0]=(me[6]-me[9])*sqrt;
			oe[1]=(me[8]-me[2])*sqrt;
			oe[2]=(me[1]-me[4])*sqrt;
			}else if ((me[0] >=me[5])&& (me[0] >=me[10])){
			sqrt=Math.sqrt(1.0+me[0]-me[5]-me[10]);
			half=0.5 / sqrt;
			oe[0]=0.5 *sqrt;
			oe[1]=(me[1]+me[4])*half;
			oe[2]=(me[2]+me[8])*half;
			oe[3]=(me[6]-me[9])*half;
			}else if (me[5] > me[10]){
			sqrt=Math.sqrt(1.0+me[5]-me[0]-me[10]);
			half=0.5 / sqrt;
			oe[0]=(me[4]+me[1])*half;
			oe[1]=0.5 *sqrt;
			oe[2]=(me[9]+me[6])*half;
			oe[3]=(me[8]-me[2])*half;
			}else {
			sqrt=Math.sqrt(1.0+me[10]-me[0]-me[5]);
			half=0.5 / sqrt;
			oe[0]=(me[8]+me[2])*half;
			oe[1]=(me[9]+me[6])*half;
			oe[2]=0.5 *sqrt;
			oe[3]=(me[1]-me[4])*half;
		}
	}

	Quaternion.slerp=function(left,right,t,out){
		var a=left.elements;
		var b=right.elements;
		var oe=out.elements;
		var ax=a[0],ay=a[1],az=a[2],aw=a[3],bx=b[0],by=b[1],bz=b[2],bw=b[3];
		var omega,cosom,sinom,scale0,scale1;
		cosom=ax *bx+ay *by+az *bz+aw *bw;
		if (cosom < 0.0){
			cosom=-cosom;
			bx=-bx;
			by=-by;
			bz=-bz;
			bw=-bw;
		}
		if ((1.0-cosom)> 0.000001){
			omega=Math.acos(cosom);
			sinom=Math.sin(omega);
			scale0=Math.sin((1.0-t)*omega)/ sinom;
			scale1=Math.sin(t *omega)/ sinom;
			}else {
			scale0=1.0-t;
			scale1=t;
		}
		oe[0]=scale0 *ax+scale1 *bx;
		oe[1]=scale0 *ay+scale1 *by;
		oe[2]=scale0 *az+scale1 *bz;
		oe[3]=scale0 *aw+scale1 *bw;
		return oe;
	}

	Quaternion.lerp=function(left,right,t,out){
		var e=out.elements;
		var f=left.elements;
		var g=right.elements;
		var ax=f[0],ay=f[1],az=f[2],aw=f[3];
		e[0]=ax+t *(g[0]-ax);
		e[1]=ay+t *(g[1]-ay);
		e[2]=az+t *(g[2]-az);
		e[3]=aw+t *(g[3]-aw);
	}

	Quaternion.add=function(left,right,out){
		var e=out.elements;
		var f=left.elements;
		var g=right.elements;
		e[0]=f[0]+g[0];
		e[1]=f[1]+g[1];
		e[2]=f[2]+g[2];
		e[3]=f[3]+g[3];
	}

	Quaternion.dot=function(left,right){
		var f=left.elements;
		var g=right.elements;
		return f[0] *g[0]+f[1] *g[1]+f[2] *g[2]+f[3] *g[3];
	}

	Quaternion.rotationLookAt=function(forward,up,out){
		Quaternion.lookAt(Vector3.ZERO,forward,up,out);
	}

	Quaternion.lookAt=function(eye,target,up,out){
		Matrix3x3.lookAt(eye,target,up,Quaternion._tempMatrix3x3);
		Quaternion.rotationMatrix(Quaternion._tempMatrix3x3,out);
	}

	Quaternion.invert=function(value,out){
		var vE=value.elements;
		var oE=out.elements;
		var lengthSq=value.lengthSquared();
		if (!MathUtils3D.isZero(lengthSq)){
			lengthSq=1.0 / lengthSq;
			oE[0]=-vE[0] *lengthSq;
			oE[1]=-vE[1] *lengthSq;
			oE[2]=-vE[2] *lengthSq;
			oE[3]=vE[3] *lengthSq;
		}
	}

	Quaternion.rotationMatrix=function(matrix3x3,out){
		var me=matrix3x3.elements;
		var m11=me[0];
		var m12=me[1];
		var m13=me[2];
		var m21=me[3];
		var m22=me[4];
		var m23=me[5];
		var m31=me[6];
		var m32=me[7];
		var m33=me[8];
		var oe=out.elements;
		var sqrt=NaN,half=NaN;
		var scale=m11+m22+m33;
		if (scale > 0){
			sqrt=Math.sqrt(scale+1);
			oe[3]=sqrt *0.5;
			sqrt=0.5 / sqrt;
			oe[0]=(m23-m32)*sqrt;
			oe[1]=(m31-m13)*sqrt;
			oe[2]=(m12-m21)*sqrt;
			}else if ((m11 >=m22)&& (m11 >=m33)){
			sqrt=Math.sqrt(1+m11-m22-m33);
			half=0.5 / sqrt;
			oe[0]=0.5 *sqrt;
			oe[1]=(m12+m21)*half;
			oe[2]=(m13+m31)*half;
			oe[3]=(m23-m32)*half;
			}else if (m22 > m33){
			sqrt=Math.sqrt(1+m22-m11-m33);
			half=0.5 / sqrt;
			oe[0]=(m21+m12)*half;
			oe[1]=0.5 *sqrt;
			oe[2]=(m32+m23)*half;
			oe[3]=(m31-m13)*half;
			}else {
			sqrt=Math.sqrt(1+m33-m11-m22);
			half=0.5 / sqrt;
			oe[0]=(m31+m13)*half;
			oe[1]=(m32+m23)*half;
			oe[2]=0.5 *sqrt;
			oe[3]=(m12-m21)*half;
		}
	}

	Quaternion.DEFAULT=new Quaternion();
	__static(Quaternion,
	['TEMPVector30',function(){return this.TEMPVector30=new Vector3();},'TEMPVector31',function(){return this.TEMPVector31=new Vector3();},'TEMPVector32',function(){return this.TEMPVector32=new Vector3();},'TEMPVector33',function(){return this.TEMPVector33=new Vector3();},'TEMPMatrix0',function(){return this.TEMPMatrix0=new Matrix4x4();},'TEMPMatrix1',function(){return this.TEMPMatrix1=new Matrix4x4();},'_tempMatrix3x3',function(){return this._tempMatrix3x3=new Matrix3x3();},'NAN',function(){return this.NAN=new Quaternion(NaN,NaN,NaN,NaN);}
	]);
	return Quaternion;
})()


/**
*<code>Ray</code> 类用于创建射线。
*/
//class laya.d3.math.Ray
var Ray=(function(){
	function Ray(origin,direction){
		/**原点*/
		this.origin=null;
		/**方向*/
		this.direction=null;
		this.origin=origin;
		this.direction=direction;
	}

	__class(Ray,'laya.d3.math.Ray');
	return Ray;
})()


/**
*<code>Vector2</code> 类用于创建二维向量。
*/
//class laya.d3.math.Vector2
var Vector2=(function(){
	function Vector2(x,y){
		this.elements=new Float32Array(2);
		(x===void 0)&& (x=0);
		(y===void 0)&& (y=0);
		var v=this.elements;
		v[0]=x;
		v[1]=y;
	}

	__class(Vector2,'laya.d3.math.Vector2');
	var __proto=Vector2.prototype;
	Laya.imps(__proto,{"laya.d3.core.IClone":true})
	/**
	*从Array数组拷贝值。
	*@param array 数组。
	*@param offset 数组偏移。
	*/
	__proto.fromArray=function(array,offset){
		(offset===void 0)&& (offset=0);
		this.elements[0]=array[offset+0];
		this.elements[1]=array[offset+1];
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		var destVector2=destObject;
		var destE=destVector2.elements;
		var s=this.elements;
		destE[0]=s[0];
		destE[1]=s[1];
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var destVector2=/*__JS__ */new this.constructor();
		this.cloneTo(destVector2);
		return destVector2;
	}

	/**
	*设置X轴坐标。
	*@param value X轴坐标。
	*/
	/**
	*获取X轴坐标。
	*@return X轴坐标。
	*/
	__getset(0,__proto,'x',function(){
		return this.elements[0];
		},function(value){
		this.elements[0]=value;
	});

	/**
	*设置Y轴坐标。
	*@param value Y轴坐标。
	*/
	/**
	*获取Y轴坐标。
	*@return Y轴坐标。
	*/
	__getset(0,__proto,'y',function(){
		return this.elements[1];
		},function(value){
		this.elements[1]=value;
	});

	Vector2.scale=function(a,b,out){
		var e=out.elements;
		var f=a.elements;
		e[0]=f[0] *b;
		e[1]=f[1] *b;
	}

	__static(Vector2,
	['ZERO',function(){return this.ZERO=new Vector2(0.0,0.0);},'ONE',function(){return this.ONE=new Vector2(1.0,1.0);}
	]);
	return Vector2;
})()


/**
*<code>Vector3</code> 类用于创建三维向量。
*/
//class laya.d3.math.Vector3
var Vector3=(function(){
	function Vector3(x,y,z){
		this.elements=new Float32Array(3);
		(x===void 0)&& (x=0);
		(y===void 0)&& (y=0);
		(z===void 0)&& (z=0);
		var v=this.elements;
		v[0]=x;
		v[1]=y;
		v[2]=z;
	}

	__class(Vector3,'laya.d3.math.Vector3');
	var __proto=Vector3.prototype;
	Laya.imps(__proto,{"laya.d3.core.IClone":true})
	/**
	*从Array数组拷贝值。
	*@param array 数组。
	*@param offset 数组偏移。
	*/
	__proto.fromArray=function(array,offset){
		(offset===void 0)&& (offset=0);
		this.elements[0]=array[offset+0];
		this.elements[1]=array[offset+1];
		this.elements[2]=array[offset+2];
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		var destVector3=destObject;
		var destE=destVector3.elements;
		var s=this.elements;
		destE[0]=s[0];
		destE[1]=s[1];
		destE[2]=s[2];
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var destVector3=/*__JS__ */new this.constructor();
		this.cloneTo(destVector3);
		return destVector3;
	}

	__proto.toDefault=function(){
		this.elements[0]=0;
		this.elements[1]=0;
		this.elements[2]=0;
	}

	/**
	*设置X轴坐标。
	*@param value X轴坐标。
	*/
	/**
	*获取X轴坐标。
	*@return X轴坐标。
	*/
	__getset(0,__proto,'x',function(){
		return this.elements[0];
		},function(value){
		this.elements[0]=value;
	});

	/**
	*设置Y轴坐标。
	*@param value Y轴坐标。
	*/
	/**
	*获取Y轴坐标。
	*@return Y轴坐标。
	*/
	__getset(0,__proto,'y',function(){
		return this.elements[1];
		},function(value){
		this.elements[1]=value;
	});

	/**
	*设置Z轴坐标。
	*@param value Z轴坐标。
	*/
	/**
	*获取Z轴坐标。
	*@return Z轴坐标。
	*/
	__getset(0,__proto,'z',function(){
		return this.elements[2];
		},function(value){
		this.elements[2]=value;
	});

	Vector3.distanceSquared=function(value1,value2){
		var value1e=value1.elements;
		var value2e=value2.elements;
		var x=value1e[0]-value2e[0];
		var y=value1e[1]-value2e[1];
		var z=value1e[2]-value2e[2];
		return (x *x)+(y *y)+(z *z);
	}

	Vector3.distance=function(value1,value2){
		var value1e=value1.elements;
		var value2e=value2.elements;
		var x=value1e[0]-value2e[0];
		var y=value1e[1]-value2e[1];
		var z=value1e[2]-value2e[2];
		return Math.sqrt((x *x)+(y *y)+(z *z));
	}

	Vector3.min=function(a,b,out){
		var e=out.elements;
		var f=a.elements;
		var g=b.elements
		e[0]=Math.min(f[0],g[0]);
		e[1]=Math.min(f[1],g[1]);
		e[2]=Math.min(f[2],g[2]);
	}

	Vector3.max=function(a,b,out){
		var e=out.elements;
		var f=a.elements;
		var g=b.elements
		e[0]=Math.max(f[0],g[0]);
		e[1]=Math.max(f[1],g[1]);
		e[2]=Math.max(f[2],g[2]);
	}

	Vector3.transformQuat=function(source,rotation,out){
		var destination=out.elements;
		var se=source.elements;
		var re=rotation.elements;
		var x=se[0],y=se[1],z=se[2],qx=re[0],qy=re[1],qz=re[2],qw=re[3],
		ix=qw *x+qy *z-qz *y,iy=qw *y+qz *x-qx *z,iz=qw *z+qx *y-qy *x,iw=-qx *x-qy *y-qz *z;
		destination[0]=ix *qw+iw *-qx+iy *-qz-iz *-qy;
		destination[1]=iy *qw+iw *-qy+iz *-qx-ix *-qz;
		destination[2]=iz *qw+iw *-qz+ix *-qy-iy *-qx;
	}

	Vector3.scalarLength=function(a){
		var f=a.elements;
		var x=f[0],y=f[1],z=f[2];
		return Math.sqrt(x *x+y *y+z *z);
	}

	Vector3.scalarLengthSquared=function(a){
		var f=a.elements;
		var x=f[0],y=f[1],z=f[2];
		return x *x+y *y+z *z;
	}

	Vector3.normalize=function(s,out){
		var se=s.elements;
		var oe=out.elements;
		var x=se[0],y=se[1],z=se[2];
		var len=x *x+y *y+z *z;
		if (len > 0){
			len=1 / Math.sqrt(len);
			oe[0]=se[0] *len;
			oe[1]=se[1] *len;
			oe[2]=se[2] *len;
		}
	}

	Vector3.multiply=function(a,b,out){
		var e=out.elements;
		var f=a.elements;
		var g=b.elements
		e[0]=f[0] *g[0];
		e[1]=f[1] *g[1];
		e[2]=f[2] *g[2];
	}

	Vector3.scale=function(a,b,out){
		var e=out.elements;
		var f=a.elements;
		e[0]=f[0] *b;
		e[1]=f[1] *b;
		e[2]=f[2] *b;
	}

	Vector3.lerp=function(a,b,t,out){
		var e=out.elements;
		var f=a.elements;
		var g=b.elements;
		var ax=f[0],ay=f[1],az=f[2];
		e[0]=ax+t *(g[0]-ax);
		e[1]=ay+t *(g[1]-ay);
		e[2]=az+t *(g[2]-az);
	}

	Vector3.transformV3ToV3=function(vector,transform,result){
		var intermediate=Vector3._tempVector4;
		Vector3.transformV3ToV4(vector,transform,intermediate);
		var intermediateElem=intermediate.elements;
		var resultElem=result.elements;
		resultElem[0]=intermediateElem[0];
		resultElem[1]=intermediateElem[1];
		resultElem[2]=intermediateElem[2];
	}

	Vector3.transformV3ToV4=function(vector,transform,result){
		var vectorElem=vector.elements;
		var vectorX=vectorElem[0];
		var vectorY=vectorElem[1];
		var vectorZ=vectorElem[2];
		var transformElem=transform.elements;
		var resultElem=result.elements;
		resultElem[0]=(vectorX *transformElem[0])+(vectorY *transformElem[4])+(vectorZ *transformElem[8])+transformElem[12];
		resultElem[1]=(vectorX *transformElem[1])+(vectorY *transformElem[5])+(vectorZ *transformElem[9])+transformElem[13];
		resultElem[2]=(vectorX *transformElem[2])+(vectorY *transformElem[6])+(vectorZ *transformElem[10])+transformElem[14];
		resultElem[3]=(vectorX *transformElem[3])+(vectorY *transformElem[7])+(vectorZ *transformElem[11])+transformElem[15];
	}

	Vector3.TransformNormal=function(normal,transform,result){
		var normalElem=normal.elements;
		var normalX=normalElem[0];
		var normalY=normalElem[1];
		var normalZ=normalElem[2];
		var transformElem=transform.elements;
		var resultElem=result.elements;
		resultElem[0]=(normalX *transformElem[0])+(normalY *transformElem[4])+(normalZ *transformElem[8]);
		resultElem[1]=(normalX *transformElem[1])+(normalY *transformElem[5])+(normalZ *transformElem[9]);
		resultElem[2]=(normalX *transformElem[2])+(normalY *transformElem[6])+(normalZ *transformElem[10]);
	}

	Vector3.transformCoordinate=function(coordinate,transform,result){
		var vectorElem=Vector3._tempVector4.elements;
		var coordinateElem=coordinate.elements;
		var coordinateX=coordinateElem[0];
		var coordinateY=coordinateElem[1];
		var coordinateZ=coordinateElem[2];
		var transformElem=transform.elements;
		vectorElem[0]=(coordinateX *transformElem[0])+(coordinateY *transformElem[4])+(coordinateZ *transformElem[8])+transformElem[12];
		vectorElem[1]=(coordinateX *transformElem[1])+(coordinateY *transformElem[5])+(coordinateZ *transformElem[9])+transformElem[13];
		vectorElem[2]=(coordinateX *transformElem[2])+(coordinateY *transformElem[6])+(coordinateZ *transformElem[10])+transformElem[14];
		vectorElem[3]=1.0 / ((coordinateX *transformElem[3])+(coordinateY *transformElem[7])+(coordinateZ *transformElem[11])+transformElem[15]);
		var resultElem=result.elements;
		resultElem[0]=vectorElem[0] *vectorElem[3];
		resultElem[1]=vectorElem[1] *vectorElem[3];
		resultElem[2]=vectorElem[2] *vectorElem[3];
	}

	Vector3.Clamp=function(value,min,max,out){
		var valuee=value.elements;
		var x=valuee[0];
		var y=valuee[1];
		var z=valuee[2];
		var mine=min.elements;
		var mineX=mine[0];
		var mineY=mine[1];
		var mineZ=mine[2];
		var maxe=max.elements;
		var maxeX=maxe[0];
		var maxeY=maxe[1];
		var maxeZ=maxe[2];
		var oute=out.elements;
		x=(x > maxeX)? maxeX :x;
		x=(x < mineX)? mineX :x;
		y=(y > maxeY)? maxeY :y;
		y=(y < mineY)? mineY :y;
		z=(z > maxeZ)? maxeZ :z;
		z=(z < mineZ)? mineZ :z;
		oute[0]=x;
		oute[1]=y;
		oute[2]=z;
	}

	Vector3.add=function(a,b,out){
		var e=out.elements;
		var f=a.elements;
		var g=b.elements
		e[0]=f[0]+g[0];
		e[1]=f[1]+g[1];
		e[2]=f[2]+g[2];
	}

	Vector3.subtract=function(a,b,o){
		var oe=o.elements;
		var ae=a.elements;
		var be=b.elements;
		oe[0]=ae[0]-be[0];
		oe[1]=ae[1]-be[1];
		oe[2]=ae[2]-be[2];
	}

	Vector3.cross=function(a,b,o){
		var ae=a.elements;
		var be=b.elements;
		var oe=o.elements;
		var ax=ae[0],ay=ae[1],az=ae[2],bx=be[0],by=be[1],bz=be[2];
		oe[0]=ay *bz-az *by;
		oe[1]=az *bx-ax *bz;
		oe[2]=ax *by-ay *bx;
	}

	Vector3.dot=function(a,b){
		var ae=a.elements;
		var be=b.elements;
		var r=(ae[0] *be[0])+(ae[1] *be[1])+(ae[2] *be[2]);
		return r;
	}

	Vector3.equals=function(a,b){
		var ae=a.elements;
		var be=b.elements;
		return MathUtils3D.nearEqual(Math.abs(ae[0]),Math.abs(be[0]))&& MathUtils3D.nearEqual(Math.abs(ae[1]),Math.abs(be[1]))&& MathUtils3D.nearEqual(Math.abs(ae[2]),Math.abs(be[2]));
	}

	Vector3.ZERO=new Vector3(0.0,0.0,0.0);
	Vector3.ONE=new Vector3(1.0,1.0,1.0);
	Vector3.NegativeUnitX=new Vector3(-1,0,0);
	Vector3.UnitX=new Vector3(1,0,0);
	Vector3.UnitY=new Vector3(0,1,0);
	Vector3.UnitZ=new Vector3(0,0,1);
	Vector3.ForwardRH=new Vector3(0,0,-1);
	Vector3.ForwardLH=new Vector3(0,0,1);
	Vector3.Up=new Vector3(0,1,0);
	Vector3.NAN=new Vector3(NaN,NaN,NaN);
	__static(Vector3,
	['_tempVector4',function(){return this._tempVector4=new Vector4();}
	]);
	return Vector3;
})()


/**
*<code>Vector4</code> 类用于创建四维向量。
*/
//class laya.d3.math.Vector4
var Vector4=(function(){
	function Vector4(x,y,z,w){
		this.elements=new Float32Array(4);
		(x===void 0)&& (x=0);
		(y===void 0)&& (y=0);
		(z===void 0)&& (z=0);
		(w===void 0)&& (w=0);
		var v=this.elements;
		v[0]=x;
		v[1]=y;
		v[2]=z;
		v[3]=w;
	}

	__class(Vector4,'laya.d3.math.Vector4');
	var __proto=Vector4.prototype;
	Laya.imps(__proto,{"laya.d3.core.IClone":true})
	/**
	*从Array数组拷贝值。
	*@param array 数组。
	*@param offset 数组偏移。
	*/
	__proto.fromArray=function(array,offset){
		(offset===void 0)&& (offset=0);
		this.elements[0]=array[offset+0];
		this.elements[1]=array[offset+1];
		this.elements[2]=array[offset+2];
		this.elements[3]=array[offset+3];
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		var destVector4=destObject;
		var destE=destVector4.elements;
		var s=this.elements;
		destE[0]=s[0];
		destE[1]=s[1];
		destE[2]=s[2];
		destE[3]=s[3];
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var destVector4=/*__JS__ */new this.constructor();
		this.cloneTo(destVector4);
		return destVector4;
	}

	/**
	*求四维向量的长度。
	*@return 长度。
	*/
	__proto.length=function(){
		return Math.sqrt(this.x *this.x+this.y *this.y+this.z *this.z+this.w *this.w);
	}

	/**
	*求四维向量长度的平方。
	*@return 长度的平方。
	*/
	__proto.lengthSquared=function(){
		return this.x *this.x+this.y *this.y+this.z *this.z+this.w *this.w;
	}

	/**
	*设置X轴坐标。
	*@param value X轴坐标。
	*/
	/**
	*获取X轴坐标。
	*@return X轴坐标。
	*/
	__getset(0,__proto,'x',function(){
		return this.elements[0];
		},function(value){
		this.elements[0]=value;
	});

	/**
	*设置Y轴坐标。
	*@param value Y轴坐标。
	*/
	/**
	*获取Y轴坐标。
	*@return Y轴坐标。
	*/
	__getset(0,__proto,'y',function(){
		return this.elements[1];
		},function(value){
		this.elements[1]=value;
	});

	/**
	*设置Z轴坐标。
	*@param value Z轴坐标。
	*/
	/**
	*获取Z轴坐标。
	*@return Z轴坐标。
	*/
	__getset(0,__proto,'z',function(){
		return this.elements[2];
		},function(value){
		this.elements[2]=value;
	});

	/**
	*设置W轴坐标。
	*@param value W轴坐标。
	*/
	/**
	*获取W轴坐标。
	*@return W轴坐标。
	*/
	__getset(0,__proto,'w',function(){
		return this.elements[3];
		},function(value){
		this.elements[3]=value;
	});

	Vector4.lerp=function(a,b,t,out){
		var e=out.elements;
		var f=a.elements;
		var g=b.elements;
		var ax=f[0],ay=f[1],az=f[2],aw=f[3];
		e[0]=ax+t *(g[0]-ax);
		e[1]=ay+t *(g[1]-ay);
		e[2]=az+t *(g[2]-az);
		e[3]=aw+t *(g[3]-aw);
	}

	Vector4.transformByM4x4=function(vector4,m4x4,out){
		var ve=vector4.elements;
		var vx=ve[0];
		var vy=ve[1];
		var vz=ve[2];
		var vw=ve[3];
		var me=m4x4.elements;
		var oe=out.elements;
		oe[0]=vx *me[0]+vy *me[4]+vz *me[8]+vw *me[12];
		oe[1]=vx *me[1]+vy *me[5]+vz *me[9]+vw *me[13];
		oe[2]=vx *me[2]+vy *me[6]+vz *me[10]+vw *me[14];
		oe[3]=vx *me[3]+vy *me[7]+vz *me[11]+vw *me[15];
	}

	Vector4.equals=function(a,b){
		var ae=a.elements;
		var be=b.elements;
		return MathUtils3D.nearEqual(Math.abs(ae[0]),Math.abs(be[0]))&& MathUtils3D.nearEqual(Math.abs(ae[1]),Math.abs(be[1]))&& MathUtils3D.nearEqual(Math.abs(ae[2]),Math.abs(be[2]))&& MathUtils3D.nearEqual(Math.abs(ae[3]),Math.abs(be[3]));
	}

	Vector4.normalize=function(s,out){
		var se=s.elements;
		var oe=out.elements;
		var len=/*if err,please use iflash.method.xmlLength()*/s.length();
		if (len > 0){
			oe[0]=se[0] *len;
			oe[1]=se[1] *len;
			oe[2]=se[2] *len;
			oe[3]=se[3] *len;
		}
	}

	Vector4.add=function(a,b,out){
		var oe=out.elements;
		var ae=a.elements;
		var be=b.elements;
		oe[0]=ae[0]+be[0];
		oe[1]=ae[1]+be[1];
		oe[2]=ae[2]+be[2];
		oe[3]=ae[3]+be[3];
	}

	Vector4.subtract=function(a,b,out){
		var oe=out.elements;
		var ae=a.elements;
		var be=b.elements;
		oe[0]=ae[0]-be[0];
		oe[1]=ae[1]-be[1];
		oe[2]=ae[2]-be[2];
		oe[3]=ae[3]-be[3];
	}

	Vector4.multiply=function(a,b,out){
		var oe=out.elements;
		var ae=a.elements;
		var be=b.elements;
		oe[0]=ae[0] *be[0];
		oe[1]=ae[1] *be[1];
		oe[2]=ae[2] *be[2];
		oe[3]=ae[3] *be[3];
	}

	Vector4.scale=function(a,b,out){
		var oe=out.elements;
		var ae=a.elements;
		oe[0]=ae[0] *b;
		oe[1]=ae[1] *b;
		oe[2]=ae[2] *b;
		oe[3]=ae[3] *b;
	}

	Vector4.Clamp=function(value,min,max,out){
		var valuee=value.elements;
		var x=valuee[0];
		var y=valuee[1];
		var z=valuee[2];
		var w=valuee[3];
		var mine=min.elements;
		var mineX=mine[0];
		var mineY=mine[1];
		var mineZ=mine[2];
		var mineW=mine[3];
		var maxe=max.elements;
		var maxeX=maxe[0];
		var maxeY=maxe[1];
		var maxeZ=maxe[2];
		var maxeW=maxe[3];
		var oute=out.elements;
		x=(x > maxeX)? maxeX :x;
		x=(x < mineX)? mineX :x;
		y=(y > maxeY)? maxeY :y;
		y=(y < mineY)? mineY :y;
		z=(z > maxeZ)? maxeZ :z;
		z=(z < mineZ)? mineZ :z;
		w=(w > maxeW)? maxeW :w;
		w=(w < mineW)? mineW :w;
		oute[0]=x;
		oute[1]=y;
		oute[2]=z;
		oute[3]=w;
	}

	Vector4.distanceSquared=function(value1,value2){
		var value1e=value1.elements;
		var value2e=value2.elements;
		var x=value1e[0]-value2e[0];
		var y=value1e[1]-value2e[1];
		var z=value1e[2]-value2e[2];
		var w=value1e[3]-value2e[3];
		return (x *x)+(y *y)+(z *z)+(w *w);
	}

	Vector4.distance=function(value1,value2){
		var value1e=value1.elements;
		var value2e=value2.elements;
		var x=value1e[0]-value2e[0];
		var y=value1e[1]-value2e[1];
		var z=value1e[2]-value2e[2];
		var w=value1e[3]-value2e[3];
		return Math.sqrt((x *x)+(y *y)+(z *z)+(w *w));
	}

	Vector4.dot=function(a,b){
		var ae=a.elements;
		var be=b.elements;
		var r=(ae[0] *be[0])+(ae[1] *be[1])+(ae[2] *be[2])+(ae[3] *be[3]);
		return r;
	}

	Vector4.min=function(a,b,out){
		var e=out.elements;
		var f=a.elements;
		var g=b.elements
		e[0]=Math.min(f[0],g[0]);
		e[1]=Math.min(f[1],g[1]);
		e[2]=Math.min(f[2],g[2]);
		e[3]=Math.min(f[3],g[3]);
	}

	Vector4.max=function(a,b,out){
		var e=out.elements;
		var f=a.elements;
		var g=b.elements
		e[0]=Math.max(f[0],g[0]);
		e[1]=Math.max(f[1],g[1]);
		e[2]=Math.max(f[2],g[2]);
		e[3]=Math.max(f[3],g[3]);
	}

	__static(Vector4,
	['ZERO',function(){return this.ZERO=new Vector4();},'ONE',function(){return this.ONE=new Vector4(1.0,1.0,1.0,1.0);},'UnitX',function(){return this.UnitX=new Vector4(1.0,0.0,0.0,0.0);},'UnitY',function(){return this.UnitY=new Vector4(0.0,1.0,0.0,0.0);},'UnitZ',function(){return this.UnitZ=new Vector4(0.0,0.0,1.0,0.0);},'UnitW',function(){return this.UnitW=new Vector4(0.0,0.0,0.0,1.0);}
	]);
	return Vector4;
})()


/**
*<code>Viewport</code> 类用于创建视口。
*/
//class laya.d3.math.Viewport
var Viewport=(function(){
	function Viewport(x,y,width,height){
		/**X轴坐标*/
		//this.x=NaN;
		/**Y轴坐标*/
		//this.y=NaN;
		/**宽度*/
		//this.width=NaN;
		/**高度*/
		//this.height=NaN;
		/**最小深度*/
		//this.minDepth=NaN;
		/**最大深度*/
		//this.maxDepth=NaN;
		this.minDepth=0.0;
		this.maxDepth=1.0;
		this.x=x;
		this.y=y;
		this.width=width;
		this.height=height;
	}

	__class(Viewport,'laya.d3.math.Viewport');
	var __proto=Viewport.prototype;
	/**
	*变换一个三维向量。
	*@param source 源三维向量。
	*@param matrix 变换矩阵。
	*@param vector 输出三维向量。
	*/
	__proto.project=function(source,matrix,out){
		Vector3.transformV3ToV3(source,matrix,out);
		var sourceEleme=source.elements;
		var matrixEleme=matrix.elements;
		var outEleme=out.elements;
		var a=(((sourceEleme[0] *matrixEleme[3])+(sourceEleme[1] *matrixEleme[7]))+(sourceEleme[2] *matrixEleme[11]))+matrixEleme[15];
		if (a!==1.0){
			outEleme[0]=outEleme[0] / a;
			outEleme[1]=outEleme[1] / a;
			outEleme[2]=outEleme[2] / a;
		}
		outEleme[0]=(((outEleme[0]+1.0)*0.5)*this.width)+this.x;
		outEleme[1]=(((-outEleme[1]+1.0)*0.5)*this.height)+this.y;
		outEleme[2]=(outEleme[2] *(this.maxDepth-this.minDepth))+this.minDepth;
	}

	/**
	*反变换一个三维向量。
	*@param source 源三维向量。
	*@param matrix 变换矩阵。
	*@param vector 输出三维向量。
	*/
	__proto.unprojectFromMat=function(source,matrix,out){
		var sourceEleme=source.elements;
		var matrixEleme=matrix.elements;
		var outEleme=out.elements;
		outEleme[0]=(((sourceEleme[0]-this.x)/ (this.width))*2.0)-1.0;
		outEleme[1]=-((((sourceEleme[1]-this.y)/ (this.height))*2.0)-1.0);
		var halfDepth=(this.maxDepth-this.minDepth)/ 2;
		outEleme[2]=(sourceEleme[2]-this.minDepth-halfDepth)/ halfDepth;
		var a=(((outEleme[0] *matrixEleme[3])+(outEleme[1] *matrixEleme[7]))+(outEleme[2] *matrixEleme[11]))+matrixEleme[15];
		Vector3.transformV3ToV3(out,matrix,out);
		if (a!==1.0){
			outEleme[0]=outEleme[0] / a;
			outEleme[1]=outEleme[1] / a;
			outEleme[2]=outEleme[2] / a;
		}
	}

	/**
	*反变换一个三维向量。
	*@param source 源三维向量。
	*@param projection 透视投影矩阵。
	*@param view 视图矩阵。
	*@param world 世界矩阵,可设置为null。
	*@param out 输出向量。
	*/
	__proto.unprojectFromWVP=function(source,projection,view,world,out){
		Matrix4x4.multiply(projection,view,Viewport._tempMatrix4x4);
		(world)&& (Matrix4x4.multiply(Viewport._tempMatrix4x4,world,Viewport._tempMatrix4x4));
		Viewport._tempMatrix4x4.invert(Viewport._tempMatrix4x4);
		this.unprojectFromMat(source,Viewport._tempMatrix4x4,out);
	}

	__static(Viewport,
	['_tempMatrix4x4',function(){return this._tempMatrix4x4=new Matrix4x4();}
	]);
	return Viewport;
})()

/**
*@private
*/
//class laya.d3.shader.ShaderDefines
var ShaderDefines$1=(function(){
	function ShaderDefines(shaderdefines){
		/**@private [只读]*/
		this.defineCounter=0;
		/**@private [只读]*/
		this.defines=null;
		if (shaderdefines){
			this.defineCounter=shaderdefines.defineCounter;
			this.defines=shaderdefines.defines.slice();
			}else {
			this.defineCounter=0;
			this.defines=[];
		}
	}

	__class(ShaderDefines,'laya.d3.shader.ShaderDefines',null,'ShaderDefines$1');
	var __proto=ShaderDefines.prototype;
	/**
	*@private
	*/
	__proto.registerDefine=function(name){
		var value=Math.pow(2,this.defineCounter++);
		this.defines[value]=name;
		return value;
	}

	return ShaderDefines;
})()


/**
*@private
*<code>ShaderInit</code> 类用于初始化内置Shader。
*/
//class laya.d3.shader.ShaderInit3D
var ShaderInit3D=(function(){
	/**
	*创建一个 <code>ShaderInit</code> 实例。
	*/
	function ShaderInit3D(){}
	__class(ShaderInit3D,'laya.d3.shader.ShaderInit3D');
	ShaderInit3D.__init__=function(){
		ShaderCompile3D._globalRegDefine("HIGHPRECISION",ShaderCompile3D.SHADERDEFINE_HIGHPRECISION);
		ShaderCompile3D._globalRegDefine("FOG",ShaderCompile3D.SHADERDEFINE_FOG);
		ShaderCompile3D._globalRegDefine("DIRECTIONLIGHT",ShaderCompile3D.SHADERDEFINE_DIRECTIONLIGHT);
		ShaderCompile3D._globalRegDefine("POINTLIGHT",ShaderCompile3D.SHADERDEFINE_POINTLIGHT);
		ShaderCompile3D._globalRegDefine("SPOTLIGHT",ShaderCompile3D.SHADERDEFINE_SPOTLIGHT);
		ShaderCompile3D._globalRegDefine("UV",ShaderCompile3D.SHADERDEFINE_UV0);
		ShaderCompile3D._globalRegDefine("COLOR",ShaderCompile3D.SHADERDEFINE_COLOR);
		ShaderCompile3D._globalRegDefine("UV1",ShaderCompile3D.SHADERDEFINE_UV1);
		ShaderCompile3D._globalRegDefine("DEPTHFOG",ShaderCompile3D.SAHDERDEFINE_DEPTHFOG);
		Shader3D.addInclude("LightHelper.glsl","\nstruct DirectionLight\n{\n vec3 Direction;\n vec3 Diffuse;\n};\n\nstruct PointLight\n{\n vec3 Diffuse;\n vec3 Attenuation;\n vec3 Position;\n float Range;\n};\n\nstruct SpotLight\n{\n vec3 Diffuse;\n vec3 Attenuation;\n vec3 Position;\n vec3 Direction;\n float Spot;\n float Range;\n};\n\n\nvec3 NormalSampleToWorldSpace(vec3 normalMapSample, vec3 unitNormal, vec3 tangent)\n{\n	vec3 normalT = 2.0*normalMapSample - 1.0;\n\n	// Build orthonormal basis.\n	vec3 N = normalize(unitNormal);\n	vec3 T = normalize(tangent- dot(tangent, N)*N);\n	vec3 B = cross(T, N);\n\n	mat3 TBN = mat3(T, B, N);\n\n	// Transform from tangent space to world space.\n	vec3 bumpedNormal = TBN*normalT;\n\n	return bumpedNormal;\n}\n\n\nvoid  computeDirectionLight(in vec3 matDif,in vec3 matAmb,in vec4 matSpe,in DirectionLight dirLight,in vec3 ambinentColor,in vec3 normal,in vec3 toEye,out vec3 dif,out vec3 amb,out vec3 spec)\n{\n	dif=vec3(0.0);//不初始化在IOS中闪烁，PC中不会闪烁\n	amb=vec3(0.0);\n	spec=vec3(0.0);\n	vec3 lightVec=-normalize(dirLight.Direction);\n	\n	amb=matAmb*ambinentColor;\n	\n	float  diffuseFactor=dot(lightVec, normal);\n	\n	if(diffuseFactor>0.0)\n	{\n	   vec3 v = reflect(-lightVec, normal);\n	   float specFactor = pow(max(dot(v, toEye), 0.0), matSpe.w);\n	   \n	   dif = diffuseFactor * matDif * dirLight.Diffuse;\n	   spec = specFactor * matSpe.rgb;\n	}\n	\n}\n\nvoid computePointLight(in vec3 matDif,in vec3 matAmb,in vec4 matSpe,in PointLight poiLight,in vec3 ambinentColor, in vec3 pos,in vec3 normal,in vec3 toEye,out vec3 dif,out vec3 amb,out vec3 spec)\n{\n	dif=vec3(0.0);\n	amb=vec3(0.0);\n	spec=vec3(0.0);\n	vec3 lightVec = poiLight.Position - pos;\n		\n	float d = length(lightVec);\n	\n	if( d > poiLight.Range )\n		return;\n		\n	lightVec /= d; \n	\n	amb = matAmb*ambinentColor;	\n\n	float diffuseFactor = dot(lightVec, normal);\n\n	if( diffuseFactor > 0.0 )\n	{\n		vec3 v= reflect(-lightVec, normal);\n		float specFactor = pow(max(dot(v, toEye), 0.0), matSpe.w);\n					\n		dif = diffuseFactor * matDif * poiLight.Diffuse;\n		spec = specFactor * matSpe.rgb;\n	}\n\n	float attenuate = 1.0 / dot(poiLight.Attenuation, vec3(1.0, d, d*d));\n\n	dif *= attenuate;\n	spec*= attenuate;\n}\n\nvoid ComputeSpotLight(in vec3 matDif,in vec3 matAmb,in vec4 matSpe,in SpotLight spoLight,in vec3 ambinentColor,in vec3 pos, in vec3 normal,in vec3 toEye,out vec3 dif,out vec3 amb,out vec3 spec)\n{\n	amb = vec3(0.0);\n	dif =vec3(0.0);\n	spec= vec3(0.0);\n	vec3 lightVec = spoLight.Position - pos;\n		\n	float d = length(lightVec);\n	\n	if( d > spoLight.Range)\n		return;\n		\n	lightVec /= d; \n	\n	amb = matAmb*ambinentColor;	\n\n	float diffuseFactor = dot(lightVec, normal);\n\n	if(diffuseFactor > 0.0)\n	{\n		vec3 v= reflect(-lightVec, normal);\n		float specFactor = pow(max(dot(v, toEye), 0.0), matSpe.w);\n					\n		dif = diffuseFactor * matDif * spoLight.Diffuse;\n		spec = specFactor * matSpe.rgb;\n	}\n	\n	float spot = pow(max(dot(-lightVec, normalize(spoLight.Direction)), 0.0), spoLight.Spot);\n\n	float attenuate = spot/dot(spoLight.Attenuation, vec3(1.0, d, d*d));\n\n	amb *= spot;\n	dif *= attenuate;\n	spec*= attenuate;\n}\n\n");
		Shader3D.addInclude("Lighting.glsl","\nstruct DirectionLight\n{\n	vec3 Color;\n	vec3 Direction;\n};\n\nstruct PointLight\n{\n	vec3 Color;\n	vec3 Position;\n	float Range;\n};\n\nstruct SpotLight\n{\n	vec3 Color;\n	vec3 Position;\n	vec3 Direction;\n	float Spot;\n	float Range;\n};\n\n// U3D中使用衰减纹理,此函数模拟并非正确\n//float U3DAttenuation(in vec3 L,in float invLightRadius)\n//{\n//	float fRatio = clamp(length(L) * invLightRadius,0.0,1.0);\n//	fRatio *= fRatio;\n//	return 1.0 / (1.0 + 25.0 * fRatio)* clamp(4.0*(1.0 - fRatio),0.0,1.0); //fade to black as if 4 pixel texture\n//} \n\n// Same as Just Cause 2 and Crysis 2 (you can read GPU Pro 1 book for more information)\nfloat BasicAttenuation(in vec3 L,in float invLightRadius)\n{\n	vec3 distance = L * invLightRadius;\n	float attenuation = clamp(1.0 - dot(distance, distance),0.0,1.0); // Equals float attenuation = saturate(1.0f - dot(L, L) / (lightRadius *  lightRadius)); 	\n	return attenuation * attenuation;\n} \n\n// Inspired on http://fools.slindev.com/viewtopic.php?f=11&t=21&view=unread#unread	\nfloat NaturalAttenuation(in vec3 L,in float invLightRadius)\n{\n	float attenuationFactor = 30.0;\n	vec3 distance = L * invLightRadius;\n	float attenuation = dot(distance, distance); // Equals float attenuation = dot(L, L) / (lightRadius *  lightRadius);\n	attenuation = 1.0 / (attenuation * attenuationFactor + 1.0);\n	// Second we move down the function therewith it reaches zero at abscissa 1:\n	attenuationFactor = 1.0 / (attenuationFactor + 1.0); //attenuationFactor contains now the value we have to subtract\n	attenuation = max(attenuation - attenuationFactor, 0.0); // The max fixes a bug.\n	// Finally we expand the equation along the y-axis so that it starts with a function value of 1 again.\n	attenuation /= 1.0 - attenuationFactor;\n	return attenuation;\n} \n\nvoid LayaAirBlinnPhongLight (in vec3 specColor,in float specColorIntensity,in vec3 normal,in vec3 gloss, in vec3 viewDir,in vec3 lightColor, in vec3 lightVec,out vec3 diffuseColor,out vec3 specularColor)\n{\n    mediump vec3 h = normalize(viewDir-lightVec);\n    lowp float ln = max (0.0, dot (-lightVec,normal));\n    float nh = max (0.0, dot (h,normal));\n	diffuseColor=lightColor * ln;\n	specularColor=lightColor *specColor*pow (nh, specColorIntensity*128.0) * gloss;\n}\n\nvoid LayaAirBlinnPhongDiectionLight (in vec3 specColor,in float specColorIntensity,in vec3 normal,in vec3 gloss, in vec3 viewDir, in DirectionLight light,out vec3 diffuseColor,out vec3 specularColor)\n{\n	vec3 lightVec=normalize(light.Direction);\n	LayaAirBlinnPhongLight(specColor,specColorIntensity,normal,gloss,viewDir,light.Color,lightVec,diffuseColor,specularColor);\n}\n\nvoid LayaAirBlinnPhongPointLight (in vec3 pos,in vec3 specColor,in float specColorIntensity,in vec3 normal,in vec3 gloss, in vec3 viewDir, in PointLight light,out vec3 diffuseColor,out vec3 specularColor)\n{\n	vec3 lightVec =  pos-light.Position;\n	//if( length(lightVec) > light.Range )\n	//	return;\n	LayaAirBlinnPhongLight(specColor,specColorIntensity,normal,gloss,viewDir,light.Color,lightVec/length(lightVec),diffuseColor,specularColor);\n	float attenuate = BasicAttenuation(lightVec, 1.0/light.Range);\n	diffuseColor *= attenuate;\n	specularColor*= attenuate;\n}\n\nvoid LayaAirBlinnPhongSpotLight (in vec3 pos,in vec3 specColor,in float specColorIntensity,in vec3 normal,in vec3 gloss, in vec3 viewDir, in SpotLight light,out vec3 diffuseColor,out vec3 specularColor)\n{\n	vec3 lightVec =  pos-light.Position;\n	//if( length(lightVec) > light.Range )\n	//	return;\n	vec3 normalLightVec=lightVec/length(lightVec);\n	LayaAirBlinnPhongLight(specColor,specColorIntensity,normal,gloss,viewDir,light.Color,normalLightVec,diffuseColor,specularColor);\n	float spot = pow(max(dot(normalLightVec, normalize(light.Direction)), 0.0), light.Spot);\n	float attenuate = spot*BasicAttenuation(lightVec, 1.0/light.Range);\n	diffuseColor *= attenuate;\n	specularColor*= attenuate;\n}\n\nvec3 NormalSampleToWorldSpace(vec3 normalMapSample, vec3 unitNormal, vec3 tangent,vec3 binormal)\n{\n	vec3 normalT =vec3(2.0*normalMapSample.x - 1.0,1.0-2.0*normalMapSample.y,2.0*normalMapSample.z - 1.0);\n	\n	// Build orthonormal basis.\n	vec3 N = normalize(unitNormal);\n	vec3 T = normalize(tangent);\n	vec3 B = normalize(binormal);\n	mat3 TBN = mat3(T, B, N);\n	\n	// Transform from tangent space to world space.\n	vec3 bumpedNormal = TBN*normalT;\n\n	return bumpedNormal;\n}\n\n\n");
		Shader3D.addInclude("ShadowHelper.glsl","uniform sampler2D u_shadowMap1;\nuniform sampler2D u_shadowMap2;\nuniform sampler2D u_shadowMap3;\nuniform vec2	  u_shadowPCFoffset;\nuniform vec4     u_shadowPSSMDistance;\nvec4 packDepth(const in float depth)\n{\n	const vec4 bitShift = vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);\n	const vec4 bitMask	= vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);\n	vec4 res = mod(depth*bitShift*vec4(255), vec4(256))/vec4(255);\n	res -= res.xxyz * bitMask;\n	return res;\n}\nfloat unpackDepth(const in vec4 rgbaDepth)\n{\n	const vec4 bitShift = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);\n	float depth = dot(rgbaDepth, bitShift);\n	return depth;\n}\nfloat tex2DPCF( sampler2D shadowMap,vec2 texcoord,vec2 invsize,float zRef )\n{\n	vec2 texelpos =texcoord / invsize;\n	vec2 lerps = fract( texelpos );\n	float sourcevals[4];\n	sourcevals[0] = float( unpackDepth(texture2D(shadowMap,texcoord)) > zRef );\n	sourcevals[1] = float( unpackDepth(texture2D(shadowMap,texcoord + vec2(invsize.x,0))) > zRef );\n	sourcevals[2] = float( unpackDepth(texture2D(shadowMap,texcoord + vec2(0,invsize.y))) > zRef );\n	sourcevals[3] = float( unpackDepth(texture2D(shadowMap,texcoord + vec2(invsize.x, invsize.y) )) > zRef );\n	return mix( mix(sourcevals[0],sourcevals[2],lerps.y),mix(sourcevals[1],sourcevals[3],lerps.y),lerps.x );\n}\nfloat getShadowPSSM3( sampler2D shadowMap1,sampler2D shadowMap2,sampler2D shadowMap3,mat4 lightShadowVP[4],vec4 pssmDistance,vec2 shadowPCFOffset,vec3 worldPos,float posViewZ,float zBias )\n{\n	float value = 1.0;\n	int nPSNum = int(posViewZ>pssmDistance.x);\n	nPSNum += int(posViewZ>pssmDistance.y);\n	nPSNum += int(posViewZ>pssmDistance.z);\n	//真SB,webgl不支持在PS中直接访问数组\n	mat4 lightVP;\n	if( nPSNum == 0 )\n	{\n		lightVP = lightShadowVP[1];\n	}\n	else if( nPSNum == 1 )\n	{\n		lightVP = lightShadowVP[2];\n	}\n	else if( nPSNum == 2 )\n	{\n		lightVP = lightShadowVP[3];\n	}\n	vec4 vLightMVPPos = lightVP * vec4(worldPos,1.0);\n	//为了效率，在CPU计算/2.0 + 0.5\n	//vec3 vText = (vLightMVPPos.xyz / vLightMVPPos.w)/2.0 + 0.5;\n	vec3 vText = vLightMVPPos.xyz / vLightMVPPos.w;\n	float fMyZ = vText.z - zBias;\n	/*\n	bvec4 bInFrustumVec = bvec4 ( vText.x >= 0.0, vText.x <= 1.0, vText.y >= 0.0, vText.y <= 1.0 );\n	bool bInFrustum = all( bInFrustumVec );\n	bvec2 bFrustumTestVec = bvec2( bInFrustum, fMyZ <= 1.0 );\n	bool bFrustumTest = all( bFrustumTestVec );\n	if ( bFrustumTest ) \n	*/\n	if( fMyZ <= 1.0 )\n	{\n		float zdepth=0.0;\n#ifdef SHADOWMAP_PCF3\n		if ( nPSNum == 0 )\n		{\n			value =  tex2DPCF( shadowMap1, vText.xy,shadowPCFOffset,fMyZ );\n			value += tex2DPCF( shadowMap1, vText.xy+vec2(shadowPCFOffset.xy),shadowPCFOffset,	fMyZ );\n			value += tex2DPCF( shadowMap1, vText.xy+vec2(shadowPCFOffset.x,0),shadowPCFOffset,	fMyZ );\n			value += tex2DPCF( shadowMap1, vText.xy+vec2(0,shadowPCFOffset.y),shadowPCFOffset,	fMyZ );\n			value = value/4.0;\n		} \n		else if( nPSNum == 1 )\n		{\n			value = tex2DPCF( shadowMap2,vText.xy,shadowPCFOffset,fMyZ);\n		}\n		else if( nPSNum == 2 )\n		{\n			vec4 color = texture2D( shadowMap3,vText.xy );\n			zdepth = unpackDepth(color);\n			value = float(fMyZ < zdepth);\n		}\n#endif\n#ifdef SHADOWMAP_PCF2\n		if ( nPSNum == 0 )\n		{\n			value = tex2DPCF( shadowMap1,vText.xy,shadowPCFOffset,fMyZ);\n		}\n		else if( nPSNum == 1 )\n		{\n			value = tex2DPCF( shadowMap2,vText.xy,shadowPCFOffset,fMyZ);\n		}\n		else if( nPSNum == 2 )\n		{\n			vec4 color = texture2D( shadowMap3,vText.xy );\n			zdepth = unpackDepth(color);\n			value = float(fMyZ < zdepth);\n		}\n\n#endif\n#ifdef SHADOWMAP_PCF1\n		if ( nPSNum == 0 )\n		{\n			value = tex2DPCF( shadowMap1,vText.xy,shadowPCFOffset,fMyZ);\n		}\n		else if( nPSNum == 1 )\n		{\n			vec4 color = texture2D( shadowMap2,vText.xy );\n			zdepth = unpackDepth(color);\n			value = float(fMyZ < zdepth);\n		}\n		else if( nPSNum == 2 )\n		{\n			vec4 color = texture2D( shadowMap3,vText.xy );\n			zdepth = unpackDepth(color);\n			value = float(fMyZ < zdepth);\n		}\n#endif\n#ifdef SHADOWMAP_PCF_NO\n		vec4 color;\n		if ( nPSNum == 0 )\n		{\n			color = texture2D( shadowMap1,vText.xy );\n		}\n		else if( nPSNum == 1 )\n		{\n			color = texture2D( shadowMap2,vText.xy );\n		}\n		else if( nPSNum == 2 )\n		{\n			color = texture2D( shadowMap3,vText.xy );\n		}\n		zdepth = unpackDepth(color);\n		value = float(fMyZ < zdepth);\n#endif\n	}\n	return value;\n}\nfloat getShadowPSSM2( sampler2D shadowMap1,sampler2D shadowMap2,mat4 lightShadowVP[4],vec4 pssmDistance,vec2 shadowPCFOffset,vec3 worldPos,float posViewZ,float zBias )\n{\n	float value = 1.0;\n	int nPSNum = int(posViewZ>pssmDistance.x);\n	nPSNum += int(posViewZ>pssmDistance.y);\n	//真SB,webgl不支持在PS中直接访问数组\n	mat4 lightVP;\n	if( nPSNum == 0 )\n	{\n		lightVP = lightShadowVP[1];\n	}\n	else if( nPSNum == 1 )\n	{\n		lightVP = lightShadowVP[2];\n	}\n	vec4 vLightMVPPos = lightVP * vec4(worldPos,1.0);\n	//为了效率，在CPU计算/2.0 + 0.5\n	//vec3 vText = (vLightMVPPos.xyz / vLightMVPPos.w)/2.0 + 0.5;\n	vec3 vText = vLightMVPPos.xyz / vLightMVPPos.w;\n	float fMyZ = vText.z - zBias;\n	/*\n	bvec4 bInFrustumVec = bvec4 ( vText.x >= 0.0, vText.x <= 1.0, vText.y >= 0.0, vText.y <= 1.0 );\n	bool bInFrustum = all( bInFrustumVec );\n	bvec2 bFrustumTestVec = bvec2( bInFrustum, fMyZ <= 1.0 );\n	bool bFrustumTest = all( bFrustumTestVec );\n	if ( bFrustumTest ) \n	*/\n	if( fMyZ <= 1.0 )\n	{\n		float zdepth=0.0;\n#ifdef SHADOWMAP_PCF3\n		if ( nPSNum == 0 )\n		{\n			value =  tex2DPCF( shadowMap1, vText.xy,shadowPCFOffset,fMyZ );\n			value += tex2DPCF( shadowMap1, vText.xy+vec2(shadowPCFOffset.xy),shadowPCFOffset,	fMyZ );\n			value += tex2DPCF( shadowMap1, vText.xy+vec2(shadowPCFOffset.x,0),shadowPCFOffset,	fMyZ );\n			value += tex2DPCF( shadowMap1, vText.xy+vec2(0,shadowPCFOffset.y),shadowPCFOffset,	fMyZ );\n			value = value/4.0;\n		}\n		else if( nPSNum == 1 )\n		{\n			value = tex2DPCF( shadowMap2,vText.xy,shadowPCFOffset,fMyZ);\n		}\n#endif\n#ifdef SHADOWMAP_PCF2\n		if ( nPSNum == 0 )\n		{\n			value = tex2DPCF( shadowMap1,vText.xy,shadowPCFOffset,fMyZ);\n		}\n		else if( nPSNum == 1 )\n		{\n			value = tex2DPCF( shadowMap2,vText.xy,shadowPCFOffset,fMyZ);\n		}\n#endif\n#ifdef SHADOWMAP_PCF1\n		if ( nPSNum == 0 )\n		{\n			value = tex2DPCF( shadowMap1,vText.xy,shadowPCFOffset,fMyZ);\n		}\n		else if( nPSNum == 1 )\n		{\n			vec4 color = texture2D( shadowMap2,vText.xy );\n			zdepth = unpackDepth(color);\n			value = float(fMyZ < zdepth);\n		}\n#endif\n#ifdef SHADOWMAP_PCF_NO\n		vec4 color;\n		if ( nPSNum == 0 )\n		{\n			color = texture2D( shadowMap1,vText.xy );\n		}\n		else if( nPSNum == 1 )\n		{\n			color = texture2D( shadowMap2,vText.xy );\n		}\n		zdepth = unpackDepth(color);\n		value = float(fMyZ < zdepth);\n#endif\n	}\n	return value;\n}\nfloat getShadowPSSM1( sampler2D shadowMap1,vec4 lightMVPPos,vec4 pssmDistance,vec2 shadowPCFOffset,float posViewZ,float zBias )\n{\n	float value = 1.0;\n	if( posViewZ < pssmDistance.x )\n	{\n		vec3 vText = lightMVPPos.xyz / lightMVPPos.w;\n		float fMyZ = vText.z - zBias;\n		/*\n		bvec4 bInFrustumVec = bvec4 ( vText.x >= 0.0, vText.x <= 1.0, vText.y >= 0.0, vText.y <= 1.0 );\n		bool bInFrustum = all( bInFrustumVec );\n		bvec2 bFrustumTestVec = bvec2( bInFrustum, fMyZ <= 1.0 );\n		bool bFrustumTest = all( bFrustumTestVec );\n		*/\n		if ( fMyZ <= 1.0 ) \n		{\n			float zdepth=0.0;\n#ifdef SHADOWMAP_PCF3\n			value =  tex2DPCF( shadowMap1, vText.xy,shadowPCFOffset,fMyZ );\n			value += tex2DPCF( shadowMap1, vText.xy+vec2(shadowPCFOffset.xy),shadowPCFOffset,fMyZ );\n			value += tex2DPCF( shadowMap1, vText.xy+vec2(shadowPCFOffset.x,0),shadowPCFOffset,fMyZ );\n			value += tex2DPCF( shadowMap1, vText.xy+vec2(0,shadowPCFOffset.y),shadowPCFOffset,fMyZ );\n			value = value/4.0;\n#endif\n#ifdef SHADOWMAP_PCF2		\n			value = tex2DPCF( shadowMap1,vText.xy,shadowPCFOffset,fMyZ);\n#endif\n#ifdef SHADOWMAP_PCF1\n			value = tex2DPCF( shadowMap1,vText.xy,shadowPCFOffset,fMyZ);\n#endif\n#ifdef SHADOWMAP_PCF_NO		\n			vec4 color = texture2D( shadowMap1,vText.xy );\n			zdepth = unpackDepth(color);\n			value = float(fMyZ < zdepth);\n#endif\n		}\n	}\n	return value;\n}");
		var vs,ps;
		
		attributeMap={
			'a_Position':/*laya.d3.graphics.VertexElementUsage.POSITION0*/0,
			'a_Color':/*laya.d3.graphics.VertexElementUsage.COLOR0*/1,
			'a_Normal':/*laya.d3.graphics.VertexElementUsage.NORMAL0*/3,
			'a_Texcoord0':/*laya.d3.graphics.VertexElementUsage.TEXTURECOORDINATE0*/2,
			'a_Texcoord1':/*laya.d3.graphics.VertexElementUsage.TEXTURECOORDINATE1*/15,
			'a_TexcoordNext0':/*laya.d3.graphics.VertexElementUsage.NEXTTEXTURECOORDINATE0*/14,
			'a_BoneWeights':/*laya.d3.graphics.VertexElementUsage.BLENDWEIGHT0*/7,
			'a_BoneIndices':/*laya.d3.graphics.VertexElementUsage.BLENDINDICES0*/6,
			'a_Tangent0':/*laya.d3.graphics.VertexElementUsage.TANGENT0*/5};
		uniformMap={
			'u_Bones':[ /*laya.d3.core.SkinnedMeshSprite3D.BONES*/0,/*laya.d3.shader.Shader3D.PERIOD_RENDERELEMENT*/0],
			'u_DiffuseTexture':[ /*laya.d3.core.material.StandardMaterial.DIFFUSETEXTURE*/1,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_SpecularTexture':[ /*laya.d3.core.material.StandardMaterial.SPECULARTEXTURE*/3,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_NormalTexture':[ /*laya.d3.core.material.StandardMaterial.NORMALTEXTURE*/2,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_AmbientTexture':[ /*laya.d3.core.material.StandardMaterial.AMBIENTTEXTURE*/5,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_ReflectTexture':[ /*laya.d3.core.material.StandardMaterial.REFLECTTEXTURE*/6,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_AlphaTestValue':[ /*laya.d3.core.material.BaseMaterial.ALPHATESTVALUE*/0,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_Albedo':[ /*laya.d3.core.material.StandardMaterial.ALBEDO*/7,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_UVMatrix':[ /*laya.d3.core.material.StandardMaterial.UVMATRIX*/13,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_UVAge':[ /*laya.d3.core.material.StandardMaterial.UVAGE*/14,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_UVAniAge':[ /*laya.d3.core.material.StandardMaterial.UVANIAGE*/8,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_MaterialDiffuse':[ /*laya.d3.core.material.StandardMaterial.MATERIALDIFFUSE*/10,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_MaterialAmbient':[ /*laya.d3.core.material.StandardMaterial.MATERIALAMBIENT*/9,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_MaterialSpecular':[ /*laya.d3.core.material.StandardMaterial.MATERIALSPECULAR*/11,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_MaterialReflect':[ /*laya.d3.core.material.StandardMaterial.MATERIALREFLECT*/12,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_TilingOffset':[ /*laya.d3.core.material.StandardMaterial.TILINGOFFSET*/15,/*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1],
			'u_WorldMat':[ /*laya.d3.core.Sprite3D.WORLDMATRIX*/0,/*laya.d3.shader.Shader3D.PERIOD_SPRITE*/2],
			'u_MvpMatrix':[ /*laya.d3.core.Sprite3D.MVPMATRIX*/1,/*laya.d3.shader.Shader3D.PERIOD_SPRITE*/2],
			'u_LightmapScaleOffset':[ /*laya.d3.core.RenderableSprite3D.LIGHTMAPSCALEOFFSET*/2,/*laya.d3.shader.Shader3D.PERIOD_SPRITE*/2],
			'u_LightMap':[ /*laya.d3.core.RenderableSprite3D.LIGHTMAP*/3,/*laya.d3.shader.Shader3D.PERIOD_SPRITE*/2],
			'u_CameraPos':[ /*laya.d3.core.BaseCamera.CAMERAPOS*/0,/*laya.d3.shader.Shader3D.PERIOD_CAMERA*/3],
			'u_FogStart':[ /*laya.d3.core.scene.Scene.FOGSTART*/1,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_FogRange':[ /*laya.d3.core.scene.Scene.FOGRANGE*/2,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_FogColor':[ /*laya.d3.core.scene.Scene.FOGCOLOR*/0,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_DirectionLight.Direction':[ /*laya.d3.core.scene.Scene.LIGHTDIRECTION*/3,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_DirectionLight.Diffuse':[ /*laya.d3.core.scene.Scene.LIGHTDIRCOLOR*/4,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_PointLight.Position':[ /*laya.d3.core.scene.Scene.POINTLIGHTPOS*/5,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_PointLight.Range':[ /*laya.d3.core.scene.Scene.POINTLIGHTRANGE*/6,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_PointLight.Attenuation':[ /*laya.d3.core.scene.Scene.POINTLIGHTATTENUATION*/7,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_PointLight.Diffuse':[ /*laya.d3.core.scene.Scene.POINTLIGHTCOLOR*/8,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_SpotLight.Position':[ /*laya.d3.core.scene.Scene.SPOTLIGHTPOS*/9,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_SpotLight.Direction':[ /*laya.d3.core.scene.Scene.SPOTLIGHTDIRECTION*/10,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_SpotLight.Range':[ /*laya.d3.core.scene.Scene.SPOTLIGHTRANGE*/12,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_SpotLight.Spot':[ /*laya.d3.core.scene.Scene.SPOTLIGHTSPOT*/11,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_SpotLight.Attenuation':[ /*laya.d3.core.scene.Scene.SPOTLIGHTATTENUATION*/13,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_SpotLight.Diffuse':[ /*laya.d3.core.scene.Scene.SPOTLIGHTCOLOR*/14,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_AmbientColor':[ /*laya.d3.core.scene.Scene.AMBIENTCOLOR*/21,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_shadowMap1':[ /*laya.d3.core.scene.Scene.SHADOWMAPTEXTURE1*/18,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_shadowMap2':[ /*laya.d3.core.scene.Scene.SHADOWMAPTEXTURE2*/19,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_shadowMap3':[ /*laya.d3.core.scene.Scene.SHADOWMAPTEXTURE3*/20,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_shadowPSSMDistance':[ /*laya.d3.core.scene.Scene.SHADOWDISTANCE*/15,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_lightShadowVP':[ /*laya.d3.core.scene.Scene.SHADOWLIGHTVIEWPROJECT*/16,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4],
			'u_shadowPCFoffset':[ /*laya.d3.core.scene.Scene.SHADOWMAPPCFOFFSET*/17,/*laya.d3.shader.Shader3D.PERIOD_SCENE*/4]};
		var SIMPLE=Shader3D.nameKey.add("SIMPLE");
		vs="attribute vec4 a_Position;\nuniform mat4 u_MvpMatrix;\n\n\n\n#if defined(DIFFUSEMAP)||((defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT))&&(defined(COLOR)&&defined(SPECULARMAP)||defined(NORMALMAP)))||(defined(LIGHTMAP)&&defined(UV))\nattribute vec2 a_Texcoord0;\nvarying vec2 v_Texcoord0;\n  #ifdef UVTRANSFORM \n  uniform mat4 u_UVMatrix;\n  #endif\n#endif\n\n#if defined(AMBIENTMAP)||(defined(LIGHTMAP)&&defined(UV1))\nattribute vec2 a_Texcoord1;\n#endif\n\n#if defined(AMBIENTMAP)||defined(LIGHTMAP)\nuniform vec4 u_LightmapScaleOffset;\nvarying vec2 v_LightMapUV;\n#endif\n\n\n#ifdef COLOR\nattribute vec4 a_Color;\nvarying vec4 v_Color;\n#endif\n\n#ifdef BONE\nattribute vec4 a_BoneIndices;\nattribute vec4 a_BoneWeights;\nconst int c_MaxBoneCount = 24;\nuniform mat4 u_Bones[c_MaxBoneCount];\n#endif\n\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(REFLECTMAP)\nattribute vec3 a_Normal;\nvarying vec3 v_Normal;\n#endif\n\n#if (defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(REFLECTMAP))&&defined(NORMALMAP)\nattribute vec3 a_Tangent0;\nvarying vec3 v_Tangent0;\n#endif\n\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(FOG)||defined(DEPTHFOG)||defined(REFLECTMAP)||defined(RECEIVESHADOW)\nuniform mat4 u_WorldMat;\nvarying vec3 v_PositionWorld;\n#endif\n\nvarying float v_posViewZ;\n#ifdef RECEIVESHADOW\n  #ifdef SHADOWMAP_PSSM1 \n  varying vec4 v_lightMVPPos;\n  uniform mat4 u_lightShadowVP[4];\n  #endif\n#endif\n\n#ifdef TILINGOFFSET\n	uniform vec4 u_TilingOffset;\n#endif\n\nvoid main_castShadow()\n{\n#ifdef BONE\n	mat4 skinTransform=mat4(0.0);\n	skinTransform += u_Bones[int(a_BoneIndices.x)] * a_BoneWeights.x;\n	skinTransform += u_Bones[int(a_BoneIndices.y)] * a_BoneWeights.y;\n	skinTransform += u_Bones[int(a_BoneIndices.z)] * a_BoneWeights.z;\n	skinTransform += u_Bones[int(a_BoneIndices.w)] * a_BoneWeights.w;\n	vec4 position=skinTransform*a_Position;\n	gl_Position = u_MvpMatrix * position;\n#else\n	gl_Position = u_MvpMatrix * a_Position;\n#endif\n \n//TODO没考虑UV动画呢\n#if defined(DIFFUSEMAP)&&defined(ALPHATEST)\n	v_Texcoord0=a_Texcoord0;\n#endif\n	v_posViewZ = gl_Position.z;\n}\n\nvoid main_normal()\n{\n#ifdef BONE\n	mat4 skinTransform=mat4(0.0);\n	skinTransform += u_Bones[int(a_BoneIndices.x)] * a_BoneWeights.x;\n	skinTransform += u_Bones[int(a_BoneIndices.y)] * a_BoneWeights.y;\n	skinTransform += u_Bones[int(a_BoneIndices.z)] * a_BoneWeights.z;\n	skinTransform += u_Bones[int(a_BoneIndices.w)] * a_BoneWeights.w;\n	vec4 position=skinTransform*a_Position;\n	gl_Position = u_MvpMatrix * position;\n#else\n	gl_Position = u_MvpMatrix * a_Position;\n#endif\n\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(REFLECTMAP)\n	mat3 worldMat;\n	#ifdef BONE\n		worldMat=mat3(u_WorldMat*skinTransform);\n	#else\n		worldMat=mat3(u_WorldMat);\n	#endif  \n	v_Normal=worldMat*a_Normal;\n	#if (defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT))&&defined(NORMALMAP)\n		v_Tangent0=worldMat*a_Tangent0;\n	#endif\n#endif\n\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(FOG)||defined(DEPTHFOG)||defined(REFLECTMAP)||defined(RECEIVESHADOW)\n	#ifdef BONE\n		v_PositionWorld=(u_WorldMat*position).xyz;\n	#else\n		v_PositionWorld=(u_WorldMat*a_Position).xyz;\n	#endif\n#endif\n\n#if defined(DIFFUSEMAP)||((defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT))&&(defined(COLOR)&&defined(SPECULARMAP)||defined(NORMALMAP)))\n	v_Texcoord0=a_Texcoord0;\n	#ifdef TILINGOFFSET\n		v_Texcoord0=(vec2(v_Texcoord0.x,v_Texcoord0.y-1.0)*u_TilingOffset.xy)+u_TilingOffset.zw;\n		v_Texcoord0=vec2(v_Texcoord0.x,v_Texcoord0.y+1.0);\n	#endif\n	#ifdef UVTRANSFORM\n		v_Texcoord0=(u_UVMatrix*vec4(v_Texcoord0,0.0,1.0)).xy;\n	#endif\n#endif\n\n#if defined(AMBIENTMAP)||defined(LIGHTMAP)\n	#ifdef SCALEOFFSETLIGHTINGMAPUV\n		#ifdef UV1\n			v_LightMapUV=vec2(a_Texcoord1.x*u_LightmapScaleOffset.x+u_LightmapScaleOffset.z,1.0+a_Texcoord1.y*u_LightmapScaleOffset.y+u_LightmapScaleOffset.w);\n		#else\n			v_LightMapUV=vec2(a_Texcoord0.x,a_Texcoord0.y-1.0)*u_LightmapScaleOffset.xy+u_LightmapScaleOffset.zw;\n		#endif \n	#else\n		#ifdef UV1\n			v_LightMapUV=a_Texcoord1;\n		#else\n			v_LightMapUV=a_Texcoord0;\n		#endif \n	#endif \n#endif\n\n#ifdef COLOR\n	v_Color=a_Color;\n#endif\n\n#ifdef RECEIVESHADOW\n	v_posViewZ = gl_Position.w;\n	#ifdef SHADOWMAP_PSSM1 \n		v_lightMVPPos = u_lightShadowVP[0] * vec4(v_PositionWorld,1.0);\n	#endif\n#endif\n}\n\nvoid main()\n{\n#ifdef CASTSHADOW\n	main_castShadow();\n#else\n	main_normal();\n#endif\n}";
		ps="#ifdef HIGHPRECISION\nprecision highp float;\n#else\nprecision mediump float;\n#endif\n\n#include \"LightHelper.glsl\";\n\nuniform vec4 u_Albedo;\n\n#ifdef ALPHATEST\nuniform float u_AlphaTestValue;\n#endif\n\n#ifdef DIFFUSEMAP\nuniform sampler2D u_DiffuseTexture;\n#endif\n\n#ifdef REFLECTMAP\nuniform samplerCube u_ReflectTexture;\nuniform vec3 u_MaterialReflect;\n#endif\n\n#if   defined(DIFFUSEMAP)||((defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT))&&(defined(COLOR)&&defined(SPECULARMAP)||defined(NORMALMAP)))\nvarying vec2 v_Texcoord0;\n#endif\n\n#if defined(AMBIENTMAP)||defined(LIGHTMAP)\nvarying vec2 v_LightMapUV;\n#endif\n#ifdef AMBIENTMAP\nuniform sampler2D u_AmbientTexture;\n#endif\n#ifdef LIGHTMAP\nuniform sampler2D u_LightMap;\n#endif\n\n#ifdef COLOR\nvarying vec4 v_Color;\n#endif\n\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)\nuniform vec3 u_MaterialDiffuse;\nuniform vec4 u_MaterialSpecular;\n  #if (defined(DIFFUSEMAP)||defined(COLOR))&&defined(SPECULARMAP) \n  uniform sampler2D u_SpecularTexture;\n  #endif\n#endif\n\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(AMBIENTMAP)||defined(LIGHTMAP)\nuniform vec3 u_MaterialAmbient;\n#endif\n\n#if defined(FOG)||defined(DEPTHFOG)\n	uniform float u_FogStart;\n	uniform float u_FogRange;\n	#ifdef ADDTIVEFOG\n	#else\n		uniform vec3 u_FogColor;\n	#endif\n#endif\n\n\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(REFLECTMAP)\nvarying vec3 v_Normal;\n#endif\n\n#if (defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT))&&defined(NORMALMAP)\nuniform sampler2D u_NormalTexture;\nvarying vec3 v_Tangent0;\n#endif\n\n#ifdef DIRECTIONLIGHT\nuniform DirectionLight u_DirectionLight;\n#endif\n\n#ifdef POINTLIGHT\nuniform PointLight u_PointLight;\n#endif\n\n#ifdef SPOTLIGHT\nuniform SpotLight u_SpotLight;\n#endif\n\nuniform vec3 u_AmbientColor;\n\n\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(FOG)||defined(DEPTHFOG)||defined(REFLECTMAP)||(defined(RECEIVESHADOW)&&(defined(SHADOWMAP_PSM2)||defined(SHADOWMAP_PSM3)))\nuniform vec3 u_CameraPos;\n#endif\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(FOG)||defined(DEPTHFOG)||defined(REFLECTMAP)\nvarying vec3 v_PositionWorld;\n#endif\n\n#include \"ShadowHelper.glsl\"\n#ifdef RECEIVESHADOW\n	#if defined(SHADOWMAP_PSSM2)||defined(SHADOWMAP_PSSM3)\n	uniform mat4 u_lightShadowVP[4];\n	#endif\n	#ifdef SHADOWMAP_PSSM1 \n	varying vec4 v_lightMVPPos;\n	#endif\n#endif\nvarying float v_posViewZ;\n\n\n\nvoid main_castShadow()\n{\n	//gl_FragColor=vec4(v_posViewZ,0.0,0.0,1.0);\n	gl_FragColor=packDepth(v_posViewZ);\n	#if defined(DIFFUSEMAP)&&defined(ALPHATEST)\n		float alpha = texture2D(u_DiffuseTexture,v_Texcoord0).w;\n		if( alpha < u_AlphaTestValue )\n		{\n			discard;\n		}\n	#endif\n}\nvoid main_normal()\n{\n#if defined(DIFFUSEMAP)&&!defined(COLOR)\n	gl_FragColor=texture2D(u_DiffuseTexture, v_Texcoord0);\n#endif \n  \n#if defined(COLOR)&&!defined(DIFFUSEMAP)\n	gl_FragColor=v_Color;\n#endif \n  \n#if defined(DIFFUSEMAP)&&defined(COLOR)\n	vec4 texColor=texture2D(u_DiffuseTexture, v_Texcoord0);\n	gl_FragColor=texColor*v_Color;\n#endif\n  \n#if !defined(DIFFUSEMAP)&&!defined(COLOR)\n	gl_FragColor=vec4(1.0,1.0,1.0,1.0);\n#endif \n    \n#ifdef ALPHATEST\n	if(gl_FragColor.a-u_AlphaTestValue<0.0)\n		discard;\n#endif\n  \n  \n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(REFLECTMAP)\n	vec3 normal;\n    #if (defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT))&&defined(NORMALMAP)\n		vec3 normalMapSample = texture2D(u_NormalTexture, v_Texcoord0).rgb;\n		normal = normalize(NormalSampleToWorldSpace(normalMapSample, v_Normal, v_Tangent0));\n	#else\n		normal = normalize(v_Normal);\n    #endif\n#endif\n	\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)\n	vec3 diffuse = vec3(0.0);\n	vec3 ambient = vec3(0.0);\n	vec3 specular= vec3(0.0);\n	vec3 dif, amb, spe;\n#endif\n  \n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)||defined(FOG)||defined(REFLECTMAP)\n	vec3 toEye;\n	#ifdef FOG\n		toEye=u_CameraPos-v_PositionWorld;\n		float toEyeLength=length(toEye);\n		toEye/=toEyeLength;\n	#else\n		toEye=normalize(u_CameraPos-v_PositionWorld);\n	#endif\n#endif\n	\n#ifdef DIRECTIONLIGHT\n	computeDirectionLight(u_MaterialDiffuse,u_MaterialAmbient,u_MaterialSpecular,u_DirectionLight,u_AmbientColor,normal,toEye, dif, amb, spe);\n	diffuse+=dif;\n	ambient+=amb;\n	specular+=spe;\n#endif\n \n#ifdef POINTLIGHT\n	computePointLight(u_MaterialDiffuse,u_MaterialAmbient,u_MaterialSpecular,u_PointLight,u_AmbientColor,v_PositionWorld,normal,toEye, dif, amb, spe);\n	diffuse+=dif;\n	ambient+=amb;\n	specular+=spe;\n#endif\n\n#ifdef SPOTLIGHT\n	ComputeSpotLight(u_MaterialDiffuse,u_MaterialAmbient,u_MaterialSpecular,u_SpotLight,u_AmbientColor,v_PositionWorld,normal,toEye, dif, amb, spe);\n	diffuse+=dif;\n	ambient+=amb;\n	specular+=spe;\n#endif\n\n#ifdef RECEIVESHADOW\n	float shadowValue = 1.0;\n	#ifdef SHADOWMAP_PSSM3\n		shadowValue = getShadowPSSM3( u_shadowMap1,u_shadowMap2,u_shadowMap3,u_lightShadowVP,u_shadowPSSMDistance,u_shadowPCFoffset,v_PositionWorld,v_posViewZ,0.001);\n	#endif\n	#ifdef SHADOWMAP_PSSM2\n		shadowValue = getShadowPSSM2( u_shadowMap1,u_shadowMap2,u_lightShadowVP,u_shadowPSSMDistance,u_shadowPCFoffset,v_PositionWorld,v_posViewZ,0.001);\n	#endif \n	#ifdef SHADOWMAP_PSSM1\n		shadowValue = getShadowPSSM1( u_shadowMap1,v_lightMVPPos,u_shadowPSSMDistance,u_shadowPCFoffset,v_posViewZ,0.001);\n	#endif\n#endif\n\n#ifdef AMBIENTMAP\n	#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)\n		gl_FragColor.rgb=gl_FragColor.rgb*(u_MaterialAmbient+texture2D(u_AmbientTexture, v_LightMapUV).rgb); \n	#else\n		#if defined(RECEIVESHADOW)\n			gl_FragColor.rgb=gl_FragColor.rgb*(u_MaterialAmbient+texture2D(u_AmbientTexture, v_LightMapUV).rgb * shadowValue);\n		#else\n			gl_FragColor.rgb=gl_FragColor.rgb*(u_MaterialAmbient+texture2D(u_AmbientTexture, v_LightMapUV).rgb); \n		#endif\n	#endif\n#endif\n\n#ifdef LIGHTMAP\n	#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)\n		gl_FragColor.rgb=gl_FragColor.rgb*(u_MaterialAmbient+texture2D(u_LightMap, v_LightMapUV).rgb); \n	#else\n		#if defined(RECEIVESHADOW)\n			gl_FragColor.rgb=gl_FragColor.rgb*(u_MaterialAmbient+texture2D(u_LightMap, v_LightMapUV).rgb * shadowValue);\n		#else\n			gl_FragColor.rgb=gl_FragColor.rgb*(u_MaterialAmbient+texture2D(u_LightMap, v_LightMapUV).rgb); \n		#endif\n	#endif\n#endif\n\ngl_FragColor=gl_FragColor*u_Albedo;\n\n#if defined(DIRECTIONLIGHT)||defined(POINTLIGHT)||defined(SPOTLIGHT)\n	#if (defined(DIFFUSEMAP)||defined(COLOR))&&defined(SPECULARMAP)\n		specular =specular*texture2D(u_SpecularTexture, v_Texcoord0).rgb;\n    #endif\n	#ifdef RECEIVESHADOW\n		gl_FragColor =vec4( gl_FragColor.rgb*(ambient + diffuse*shadowValue) + specular*shadowValue,gl_FragColor.a);\n	#else\n		gl_FragColor =vec4( gl_FragColor.rgb*(ambient + diffuse) + specular,gl_FragColor.a);\n	#endif\n#endif\n  \n#ifdef REFLECTMAP\n	vec3 incident = -toEye;\n	vec3 reflectionVector = reflect(incident,normal);\n	vec3 reflectionColor  = textureCube(u_ReflectTexture,reflectionVector).rgb;\n	gl_FragColor.rgb += u_MaterialReflect*reflectionColor;\n#endif\n  \n#ifdef FOG\n	float lerpFact=clamp((toEyeLength-u_FogStart)/u_FogRange,0.0,1.0);\n	#ifdef ADDTIVEFOG\n		gl_FragColor.rgb=mix(gl_FragColor.rgb,vec3(0.0,0.0,0.0),lerpFact);\n	#else\n		gl_FragColor.rgb=mix(gl_FragColor.rgb,u_FogColor,lerpFact);\n	#endif\n#endif\n#ifdef DEPTHFOG\n	float lerpFact = (-v_PositionWorld.y-u_FogStart)/u_FogRange;\n	gl_FragColor.rgb=mix(gl_FragColor.rgb,u_FogColor,lerpFact);\n#endif\n}\n\nvoid main()\n{\n#ifdef CASTSHADOW		\n	main_castShadow();\n#else\n  main_normal();\n#endif  \n}\n\n";
		shaderCompile=ShaderCompile3D.add(SIMPLE,vs,ps,attributeMap,uniformMap);
		StandardMaterial.SHADERDEFINE_DIFFUSEMAP=shaderCompile.registerMaterialDefine("DIFFUSEMAP");
		StandardMaterial.SHADERDEFINE_NORMALMAP=shaderCompile.registerMaterialDefine("NORMALMAP");
		StandardMaterial.SHADERDEFINE_SPECULARMAP=shaderCompile.registerMaterialDefine("SPECULARMAP");
		StandardMaterial.SHADERDEFINE_EMISSIVEMAP=shaderCompile.registerMaterialDefine("EMISSIVEMAP");
		StandardMaterial.SHADERDEFINE_AMBIENTMAP=shaderCompile.registerMaterialDefine("AMBIENTMAP");
		StandardMaterial.SHADERDEFINE_REFLECTMAP=shaderCompile.registerMaterialDefine("REFLECTMAP");
		StandardMaterial.SHADERDEFINE_UVTRANSFORM=shaderCompile.registerMaterialDefine("UVTRANSFORM");
		StandardMaterial.SHADERDEFINE_TILINGOFFSET=shaderCompile.registerMaterialDefine("TILINGOFFSET");
		StandardMaterial.SHADERDEFINE_ADDTIVEFOG=shaderCompile.registerMaterialDefine("ADDTIVEFOG");		
	}

	return ShaderInit3D;
})()


/**
*@private
*<code>Shader3D</code> 主要用数组的方式保存shader变量定义，后期合并ShaderValue不使用for in，性能较高。
*/
//class laya.d3.shader.ValusArray
var ValusArray=(function(){
	function ValusArray(){
		this._data=null;
		this._data=[];
	}

	__class(ValusArray,'laya.d3.shader.ValusArray');
	var __proto=ValusArray.prototype;
	__proto.setValue=function(name,value){
		this._data[name]=value;
	}

	__getset(0,__proto,'data',function(){
		return this._data;
	});

	return ValusArray;
})()


//class laya.d3.utils.Size
var Size=(function(){
	function Size(width,height){
		this._width=0;
		this._height=0;
		this._width=width;
		this._height=height;
	}

	__class(Size,'laya.d3.utils.Size');
	var __proto=Size.prototype;
	__getset(0,__proto,'width',function(){
		if (this._width===-1)
			return RenderState.clientWidth;
		return this._width;
	});

	__getset(0,__proto,'height',function(){
		if (this._height===-1)
			return RenderState.clientHeight;
		return this._height;
	});

	__getset(1,Size,'fullScreen',function(){
		return new Size(-1,-1);
	});

	return Size;
})()


/**
*<code>Utils3D</code> 类用于创建3D工具。
*/
//class laya.d3.utils.Utils3D
var Utils3D=(function(){
	function Utils3D(){}
	__class(Utils3D,'laya.d3.utils.Utils3D');
	Utils3D._rotationTransformScaleSkinAnimation=function(tx,ty,tz,qx,qy,qz,qw,sx,sy,sz,outArray,outOffset){
		var re=Utils3D._tempArray16_0;
		var se=Utils3D._tempArray16_1;
		var tse=Utils3D._tempArray16_2;
		var x2=qx+qx;
		var y2=qy+qy;
		var z2=qz+qz;
		var xx=qx *x2;
		var yx=qy *x2;
		var yy=qy *y2;
		var zx=qz *x2;
		var zy=qz *y2;
		var zz=qz *z2;
		var wx=qw *x2;
		var wy=qw *y2;
		var wz=qw *z2;
		re[15]=1;
		re[0]=1-yy-zz;
		re[1]=yx+wz;
		re[2]=zx-wy;
		re[4]=yx-wz;
		re[5]=1-xx-zz;
		re[6]=zy+wx;
		re[8]=zx+wy;
		re[9]=zy-wx;
		re[10]=1-xx-yy;
		se[15]=1;
		se[0]=sx;
		se[5]=sy;
		se[10]=sz;
		var i,a,b,e,ai0,ai1,ai2,ai3;
		for (i=0;i < 4;i++){
			ai0=re[i];
			ai1=re[i+4];
			ai2=re[i+8];
			ai3=re[i+12];
			tse[i]=ai0;
			tse[i+4]=ai1;
			tse[i+8]=ai2;
			tse[i+12]=ai0 *tx+ai1 *ty+ai2 *tz+ai3;
		}
		for (i=0;i < 4;i++){
			ai0=tse[i];
			ai1=tse[i+4];
			ai2=tse[i+8];
			ai3=tse[i+12];
			outArray[i+outOffset]=ai0 *se[0]+ai1 *se[1]+ai2 *se[2]+ai3 *se[3];
			outArray[i+outOffset+4]=ai0 *se[4]+ai1 *se[5]+ai2 *se[6]+ai3 *se[7];
			outArray[i+outOffset+8]=ai0 *se[8]+ai1 *se[9]+ai2 *se[10]+ai3 *se[11];
			outArray[i+outOffset+12]=ai0 *se[12]+ai1 *se[13]+ai2 *se[14]+ai3 *se[15];
		}
	}

	Utils3D._createNodeByJson=function(rootNode,nodeData,node,innerResouMap){
		if (!node){
			switch (nodeData.type){
				case "Sprite3D":
					node=new Sprite3D();
					break ;
				case "MeshSprite3D":
					node=new MeshSprite3D();
					break ;
				case "SkinnedMeshSprite3D":
					node=new SkinnedMeshSprite3D();
					break ;
				case "ShuriKenParticle3D":
					node=new ShuriKenParticle3D();
					break ;
				case "TrailSprite3D":
					node=new TrailSprite3D();
					break ;
				case "LineSprite3D":
					node=new LineSprite3D();
					break ;
				case "Terrain":
					node=new Terrain();
					break ;
				case "Camera":
					node=new Camera();
					break ;
				case "DirectionLight":
					node=new DirectionLight();
					break ;
				default :
					throw new Error("Utils3D:unidentified class type in (.lh) file.");
				}
		};
		var props=nodeData.props;
		if (props)
			for (var key in props)
		node[key]=props[key];
		var customProps=nodeData.customProps;
		if (customProps){
			if ((node instanceof laya.d3.core.Sprite3D )){
				node._parseBaseCustomProps(customProps);
				node._parseCustomProps(rootNode,innerResouMap,customProps,nodeData);
				node._parseCustomComponent(rootNode,innerResouMap,nodeData.components);
				}else {
				node._parseCustomProps(rootNode,innerResouMap,customProps,nodeData);
			}
		};
		var childData=nodeData.child;
		if (childData){
			for (var i=0,n=childData.length;i < n;i++){
				var child=Utils3D._createNodeByJson(rootNode,childData[i],null,innerResouMap)
				node.addChild(child);
			}
		}
		return node;
	}

	Utils3D._computeBoneAndAnimationDatasByBindPoseMatrxix=function(bones,curData,inverGlobalBindPose,outBonesDatas,outAnimationDatas,boneIndexToMesh){
		var offset=0;
		var matOffset=0;
		var i;
		var parentOffset;
		var boneLength=bones.length;
		for (i=0;i < boneLength;offset+=bones[i].keyframeWidth,matOffset+=16,i++){
			laya.d3.utils.Utils3D._rotationTransformScaleSkinAnimation(curData[offset+0],curData[offset+1],curData[offset+2],curData[offset+3],curData[offset+4],curData[offset+5],curData[offset+6],curData[offset+7],curData[offset+8],curData[offset+9],outBonesDatas,matOffset);
			if (i !=0){
				parentOffset=bones[i].parentIndex *16;
				laya.d3.utils.Utils3D.mulMatrixByArray(outBonesDatas,parentOffset,outBonesDatas,matOffset,outBonesDatas,matOffset);
			}
		};
		var n=inverGlobalBindPose.length;
		for (i=0;i < n;i++){
			laya.d3.utils.Utils3D.mulMatrixByArrayAndMatrixFast(outBonesDatas,boneIndexToMesh[i] *16,inverGlobalBindPose[i],outAnimationDatas,i *16);
		}
	}

	Utils3D._computeAnimationDatasByArrayAndMatrixFast=function(inverGlobalBindPose,bonesDatas,outAnimationDatas,boneIndexToMesh){
		for (var i=0,n=inverGlobalBindPose.length;i < n;i++)
		laya.d3.utils.Utils3D.mulMatrixByArrayAndMatrixFast(bonesDatas,boneIndexToMesh[i] *16,inverGlobalBindPose[i],outAnimationDatas,i *16);
	}

	Utils3D._computeBoneAndAnimationDatasByBindPoseMatrxixOld=function(bones,curData,inverGlobalBindPose,outBonesDatas,outAnimationDatas){
		var offset=0;
		var matOffset=0;
		var i;
		var parentOffset;
		var boneLength=bones.length;
		for (i=0;i < boneLength;offset+=bones[i].keyframeWidth,matOffset+=16,i++){
			laya.d3.utils.Utils3D._rotationTransformScaleSkinAnimation(curData[offset+7],curData[offset+8],curData[offset+9],curData[offset+3],curData[offset+4],curData[offset+5],curData[offset+6],curData[offset+0],curData[offset+1],curData[offset+2],outBonesDatas,matOffset);
			if (i !=0){
				parentOffset=bones[i].parentIndex *16;
				laya.d3.utils.Utils3D.mulMatrixByArray(outBonesDatas,parentOffset,outBonesDatas,matOffset,outBonesDatas,matOffset);
			}
		};
		var n=inverGlobalBindPose.length;
		for (i=0;i < n;i++){
			var arrayOffset=i *16;
			laya.d3.utils.Utils3D.mulMatrixByArrayAndMatrixFast(outBonesDatas,arrayOffset,inverGlobalBindPose[i],outAnimationDatas,arrayOffset);
		}
	}

	Utils3D._computeAnimationDatasByArrayAndMatrixFastOld=function(inverGlobalBindPose,bonesDatas,outAnimationDatas){
		var n=inverGlobalBindPose.length;
		for (var i=0;i < n;i++){
			var arrayOffset=i *16;
			laya.d3.utils.Utils3D.mulMatrixByArrayAndMatrixFast(bonesDatas,arrayOffset,inverGlobalBindPose[i],outAnimationDatas,arrayOffset);
		}
	}

	Utils3D._computeRootAnimationData=function(bones,curData,animationDatas){
		for (var i=0,offset=0,matOffset=0,boneLength=bones.length;i < boneLength;offset+=bones[i].keyframeWidth,matOffset+=16,i++)
		laya.d3.utils.Utils3D.createAffineTransformationArray(curData[offset+0],curData[offset+1],curData[offset+2],curData[offset+3],curData[offset+4],curData[offset+5],curData[offset+6],curData[offset+7],curData[offset+8],curData[offset+9],animationDatas,matOffset);
	}

	Utils3D.transformVector3ArrayByQuat=function(sourceArray,sourceOffset,rotation,outArray,outOffset){
		var re=rotation.elements;
		var x=sourceArray[sourceOffset],y=sourceArray[sourceOffset+1],z=sourceArray[sourceOffset+2],qx=re[0],qy=re[1],qz=re[2],qw=re[3],ix=qw *x+qy *z-qz *y,iy=qw *y+qz *x-qx *z,iz=qw *z+qx *y-qy *x,iw=-qx *x-qy *y-qz *z;
		outArray[outOffset]=ix *qw+iw *-qx+iy *-qz-iz *-qy;
		outArray[outOffset+1]=iy *qw+iw *-qy+iz *-qx-ix *-qz;
		outArray[outOffset+2]=iz *qw+iw *-qz+ix *-qy-iy *-qx;
	}

	Utils3D.mulMatrixByArray=function(leftArray,leftOffset,rightArray,rightOffset,outArray,outOffset){
		var i,ai0,ai1,ai2,ai3;
		if (outArray===rightArray){
			rightArray=Utils3D._tempArray16_3;
			for (i=0;i < 16;++i){
				rightArray[i]=outArray[outOffset+i];
			}
			rightOffset=0;
		}
		for (i=0;i < 4;i++){
			ai0=leftArray[leftOffset+i];
			ai1=leftArray[leftOffset+i+4];
			ai2=leftArray[leftOffset+i+8];
			ai3=leftArray[leftOffset+i+12];
			outArray[outOffset+i]=ai0 *rightArray[rightOffset+0]+ai1 *rightArray[rightOffset+1]+ai2 *rightArray[rightOffset+2]+ai3 *rightArray[rightOffset+3];
			outArray[outOffset+i+4]=ai0 *rightArray[rightOffset+4]+ai1 *rightArray[rightOffset+5]+ai2 *rightArray[rightOffset+6]+ai3 *rightArray[rightOffset+7];
			outArray[outOffset+i+8]=ai0 *rightArray[rightOffset+8]+ai1 *rightArray[rightOffset+9]+ai2 *rightArray[rightOffset+10]+ai3 *rightArray[rightOffset+11];
			outArray[outOffset+i+12]=ai0 *rightArray[rightOffset+12]+ai1 *rightArray[rightOffset+13]+ai2 *rightArray[rightOffset+14]+ai3 *rightArray[rightOffset+15];
		}
	}

	Utils3D.mulMatrixByArrayFast=function(leftArray,leftOffset,rightArray,rightOffset,outArray,outOffset){
		var i,ai0,ai1,ai2,ai3;
		for (i=0;i < 4;i++){
			ai0=leftArray[leftOffset+i];
			ai1=leftArray[leftOffset+i+4];
			ai2=leftArray[leftOffset+i+8];
			ai3=leftArray[leftOffset+i+12];
			outArray[outOffset+i]=ai0 *rightArray[rightOffset+0]+ai1 *rightArray[rightOffset+1]+ai2 *rightArray[rightOffset+2]+ai3 *rightArray[rightOffset+3];
			outArray[outOffset+i+4]=ai0 *rightArray[rightOffset+4]+ai1 *rightArray[rightOffset+5]+ai2 *rightArray[rightOffset+6]+ai3 *rightArray[rightOffset+7];
			outArray[outOffset+i+8]=ai0 *rightArray[rightOffset+8]+ai1 *rightArray[rightOffset+9]+ai2 *rightArray[rightOffset+10]+ai3 *rightArray[rightOffset+11];
			outArray[outOffset+i+12]=ai0 *rightArray[rightOffset+12]+ai1 *rightArray[rightOffset+13]+ai2 *rightArray[rightOffset+14]+ai3 *rightArray[rightOffset+15];
		}
	}

	Utils3D.mulMatrixByArrayAndMatrixFast=function(leftArray,leftOffset,rightMatrix,outArray,outOffset){
		var i,ai0,ai1,ai2,ai3;
		var rightMatrixE=rightMatrix.elements;
		var m11=rightMatrixE[0],m12=rightMatrixE[1],m13=rightMatrixE[2],m14=rightMatrixE[3];
		var m21=rightMatrixE[4],m22=rightMatrixE[5],m23=rightMatrixE[6],m24=rightMatrixE[7];
		var m31=rightMatrixE[8],m32=rightMatrixE[9],m33=rightMatrixE[10],m34=rightMatrixE[11];
		var m41=rightMatrixE[12],m42=rightMatrixE[13],m43=rightMatrixE[14],m44=rightMatrixE[15];
		var ai0LeftOffset=leftOffset;
		var ai1LeftOffset=leftOffset+4;
		var ai2LeftOffset=leftOffset+8;
		var ai3LeftOffset=leftOffset+12;
		var ai0OutOffset=outOffset;
		var ai1OutOffset=outOffset+4;
		var ai2OutOffset=outOffset+8;
		var ai3OutOffset=outOffset+12;
		for (i=0;i < 4;i++){
			ai0=leftArray[ai0LeftOffset+i];
			ai1=leftArray[ai1LeftOffset+i];
			ai2=leftArray[ai2LeftOffset+i];
			ai3=leftArray[ai3LeftOffset+i];
			outArray[ai0OutOffset+i]=ai0 *m11+ai1 *m12+ai2 *m13+ai3 *m14;
			outArray[ai1OutOffset+i]=ai0 *m21+ai1 *m22+ai2 *m23+ai3 *m24;
			outArray[ai2OutOffset+i]=ai0 *m31+ai1 *m32+ai2 *m33+ai3 *m34;
			outArray[ai3OutOffset+i]=ai0 *m41+ai1 *m42+ai2 *m43+ai3 *m44;
		}
	}

	Utils3D.createAffineTransformationArray=function(tX,tY,tZ,rX,rY,rZ,rW,sX,sY,sZ,outArray,outOffset){
		var x2=rX+rX,y2=rY+rY,z2=rZ+rZ;
		var xx=rX *x2,xy=rX *y2,xz=rX *z2,yy=rY *y2,yz=rY *z2,zz=rZ *z2;
		var wx=rW *x2,wy=rW *y2,wz=rW *z2;
		outArray[outOffset+0]=(1-(yy+zz))*sX;
		outArray[outOffset+1]=(xy+wz)*sX;
		outArray[outOffset+2]=(xz-wy)*sX;
		outArray[outOffset+3]=0;
		outArray[outOffset+4]=(xy-wz)*sY;
		outArray[outOffset+5]=(1-(xx+zz))*sY;
		outArray[outOffset+6]=(yz+wx)*sY;
		outArray[outOffset+7]=0;
		outArray[outOffset+8]=(xz+wy)*sZ;
		outArray[outOffset+9]=(yz-wx)*sZ;
		outArray[outOffset+10]=(1-(xx+yy))*sZ;
		outArray[outOffset+11]=0;
		outArray[outOffset+12]=tX;
		outArray[outOffset+13]=tY;
		outArray[outOffset+14]=tZ;
		outArray[outOffset+15]=1;
	}

	Utils3D.transformVector3ArrayToVector3ArrayCoordinate=function(source,sourceOffset,transform,result,resultOffset){
		var vectorElem=Utils3D._tempArray4_0;
		var coordinateX=source[sourceOffset+0];
		var coordinateY=source[sourceOffset+1];
		var coordinateZ=source[sourceOffset+2];
		var transformElem=transform.elements;
		vectorElem[0]=(coordinateX *transformElem[0])+(coordinateY *transformElem[4])+(coordinateZ *transformElem[8])+transformElem[12];
		vectorElem[1]=(coordinateX *transformElem[1])+(coordinateY *transformElem[5])+(coordinateZ *transformElem[9])+transformElem[13];
		vectorElem[2]=(coordinateX *transformElem[2])+(coordinateY *transformElem[6])+(coordinateZ *transformElem[10])+transformElem[14];
		vectorElem[3]=1.0 / ((coordinateX *transformElem[3])+(coordinateY *transformElem[7])+(coordinateZ *transformElem[11])+transformElem[15]);
		result[resultOffset+0]=vectorElem[0] *vectorElem[3];
		result[resultOffset+1]=vectorElem[1] *vectorElem[3];
		result[resultOffset+2]=vectorElem[2] *vectorElem[3];
	}

	Utils3D.transformLightingMapTexcoordByUV0Array=function(source,sourceOffset,lightingMapScaleOffset,result,resultOffset){
		var lightingMapScaleOffsetE=lightingMapScaleOffset.elements;
		result[resultOffset+0]=source[sourceOffset+0] *lightingMapScaleOffsetE[0]+lightingMapScaleOffsetE[2];
		result[resultOffset+1]=(source[sourceOffset+1]-1.0)*lightingMapScaleOffsetE[1]+lightingMapScaleOffsetE[3];
	}

	Utils3D.transformLightingMapTexcoordByUV1Array=function(source,sourceOffset,lightingMapScaleOffset,result,resultOffset){
		var lightingMapScaleOffsetE=lightingMapScaleOffset.elements;
		result[resultOffset+0]=source[sourceOffset+0] *lightingMapScaleOffsetE[0]+lightingMapScaleOffsetE[2];
		result[resultOffset+1]=1.0+source[sourceOffset+1] *lightingMapScaleOffsetE[1]+lightingMapScaleOffsetE[3];
	}

	Utils3D.getURLVerion=function(url){
		var index=url.indexOf("?");
		return index >=0 ? url.substr(index):null;
	}

	Utils3D._quaternionCreateFromYawPitchRollArray=function(yaw,pitch,roll,out){
		var halfRoll=roll *0.5;
		var halfPitch=pitch *0.5;
		var halfYaw=yaw *0.5;
		var sinRoll=Math.sin(halfRoll);
		var cosRoll=Math.cos(halfRoll);
		var sinPitch=Math.sin(halfPitch);
		var cosPitch=Math.cos(halfPitch);
		var sinYaw=Math.sin(halfYaw);
		var cosYaw=Math.cos(halfYaw);
		out[0]=(cosYaw *sinPitch *cosRoll)+(sinYaw *cosPitch *sinRoll);
		out[1]=(sinYaw *cosPitch *cosRoll)-(cosYaw *sinPitch *sinRoll);
		out[2]=(cosYaw *cosPitch *sinRoll)-(sinYaw *sinPitch *cosRoll);
		out[3]=(cosYaw *cosPitch *cosRoll)+(sinYaw *sinPitch *sinRoll);
	}

	Utils3D._createAffineTransformationArray=function(trans,rot,scale,outE){
		var x=rot[0],y=rot[1],z=rot[2],w=rot[3],x2=x+x,y2=y+y,z2=z+z;
		var xx=x *x2,xy=x *y2,xz=x *z2,yy=y *y2,yz=y *z2,zz=z *z2;
		var wx=w *x2,wy=w *y2,wz=w *z2,sx=scale[0],sy=scale[1],sz=scale[2];
		outE[0]=(1-(yy+zz))*sx;
		outE[1]=(xy+wz)*sx;
		outE[2]=(xz-wy)*sx;
		outE[3]=0;
		outE[4]=(xy-wz)*sy;
		outE[5]=(1-(xx+zz))*sy;
		outE[6]=(yz+wx)*sy;
		outE[7]=0;
		outE[8]=(xz+wy)*sz;
		outE[9]=(yz-wx)*sz;
		outE[10]=(1-(xx+yy))*sz;
		outE[11]=0;
		outE[12]=trans[0];
		outE[13]=trans[1];
		outE[14]=trans[2];
		outE[15]=1;
	}

	Utils3D._mulMatrixArray=function(leftMatrixE,rightMatrix,outArray,outOffset){
		var i,ai0,ai1,ai2,ai3;
		var rightMatrixE=rightMatrix.elements;
		var m11=rightMatrixE[0],m12=rightMatrixE[1],m13=rightMatrixE[2],m14=rightMatrixE[3];
		var m21=rightMatrixE[4],m22=rightMatrixE[5],m23=rightMatrixE[6],m24=rightMatrixE[7];
		var m31=rightMatrixE[8],m32=rightMatrixE[9],m33=rightMatrixE[10],m34=rightMatrixE[11];
		var m41=rightMatrixE[12],m42=rightMatrixE[13],m43=rightMatrixE[14],m44=rightMatrixE[15];
		var ai0OutOffset=outOffset;
		var ai1OutOffset=outOffset+4;
		var ai2OutOffset=outOffset+8;
		var ai3OutOffset=outOffset+12;
		for (i=0;i < 4;i++){
			ai0=leftMatrixE[i];
			ai1=leftMatrixE[i+4];
			ai2=leftMatrixE[i+8];
			ai3=leftMatrixE[i+12];
			outArray[ai0OutOffset+i]=ai0 *m11+ai1 *m12+ai2 *m13+ai3 *m14;
			outArray[ai1OutOffset+i]=ai0 *m21+ai1 *m22+ai2 *m23+ai3 *m24;
			outArray[ai2OutOffset+i]=ai0 *m31+ai1 *m32+ai2 *m33+ai3 *m34;
			outArray[ai3OutOffset+i]=ai0 *m41+ai1 *m42+ai2 *m43+ai3 *m44;
		}
	}

	Utils3D.getYawPitchRoll=function(quaternion,out){
		Utils3D.transformQuat(Vector3.ForwardRH,quaternion,Quaternion.TEMPVector31);
		Utils3D.transformQuat(Vector3.Up,quaternion,Quaternion.TEMPVector32);
		var upe=Quaternion.TEMPVector32.elements;
		Utils3D.angleTo(Vector3.ZERO,Quaternion.TEMPVector31,Quaternion.TEMPVector33);
		var anglee=Quaternion.TEMPVector33.elements;
		if (anglee[0]==Math.PI / 2){
			anglee[1]=Utils3D.arcTanAngle(upe[2],upe[0]);
			anglee[2]=0;
			}else if (anglee[0]==-Math.PI / 2){
			anglee[1]=Utils3D.arcTanAngle(-upe[2],-upe[0]);
			anglee[2]=0;
			}else {
			Matrix4x4.createRotationY(-anglee[1],Quaternion.TEMPMatrix0);
			Matrix4x4.createRotationX(-anglee[0],Quaternion.TEMPMatrix1);
			Vector3.transformCoordinate(Quaternion.TEMPVector32,Quaternion.TEMPMatrix0,Quaternion.TEMPVector32);
			Vector3.transformCoordinate(Quaternion.TEMPVector32,Quaternion.TEMPMatrix1,Quaternion.TEMPVector32);
			anglee[2]=Utils3D.arcTanAngle(upe[1],-upe[0]);
		}
		if (anglee[1] <=-Math.PI)
			anglee[1]=Math.PI;
		if (anglee[2] <=-Math.PI)
			anglee[2]=Math.PI;
		if (anglee[1] >=Math.PI && anglee[2] >=Math.PI){
			anglee[1]=0;
			anglee[2]=0;
			anglee[0]=Math.PI-anglee[0];
		}
		out[0]=anglee[1];
		out[1]=anglee[0];
		out[2]=anglee[2];
	}

	Utils3D.arcTanAngle=function(x,y){
		if (x==0){
			if (y==1)
				return Math.PI / 2;
			return-Math.PI / 2;
		}
		if (x > 0)
			return Math.atan(y / x);
		if (x < 0){
			if (y > 0)
				return Math.atan(y / x)+Math.PI;
			return Math.atan(y / x)-Math.PI;
		}
		return 0;
	}

	Utils3D.angleTo=function(from,location,angle){
		Vector3.subtract(location,from,Quaternion.TEMPVector30);
		Vector3.normalize(Quaternion.TEMPVector30,Quaternion.TEMPVector30);
		angle.elements[0]=Math.asin(Quaternion.TEMPVector30.y);
		angle.elements[1]=Utils3D.arcTanAngle(-Quaternion.TEMPVector30.z,-Quaternion.TEMPVector30.x);
	}

	Utils3D.transformQuat=function(source,rotation,out){
		var destination=out.elements;
		var se=source.elements;
		var re=rotation;
		var x=se[0],y=se[1],z=se[2],qx=re[0],qy=re[1],qz=re[2],qw=re[3],
		ix=qw *x+qy *z-qz *y,iy=qw *y+qz *x-qx *z,iz=qw *z+qx *y-qy *x,iw=-qx *x-qy *y-qz *z;
		destination[0]=ix *qw+iw *-qx+iy *-qz-iz *-qy;
		destination[1]=iy *qw+iw *-qy+iz *-qx-ix *-qz;
		destination[2]=iz *qw+iw *-qz+ix *-qy-iy *-qx;
	}

	Utils3D.quaterionNormalize=function(f,e){
		var x=f[0],y=f[1],z=f[2],w=f[3];
		var len=x *x+y *y+z *z+w *w;
		if (len > 0){
			len=1 / Math.sqrt(len);
			e[0]=x *len;
			e[1]=y *len;
			e[2]=z *len;
			e[3]=w *len;
		}
	}

	Utils3D.matrix4x4MultiplyFFF=function(a,b,e){
		var i,ai0,ai1,ai2,ai3;
		if (e===b){
			b=new Float32Array(16);
			for (i=0;i < 16;++i){
				b[i]=e[i];
			}
		}
		for (i=0;i < 4;i++){
			ai0=a[i];
			ai1=a[i+4];
			ai2=a[i+8];
			ai3=a[i+12];
			e[i]=ai0 *b[0]+ai1 *b[1]+ai2 *b[2]+ai3 *b[3];
			e[i+4]=ai0 *b[4]+ai1 *b[5]+ai2 *b[6]+ai3 *b[7];
			e[i+8]=ai0 *b[8]+ai1 *b[9]+ai2 *b[10]+ai3 *b[11];
			e[i+12]=ai0 *b[12]+ai1 *b[13]+ai2 *b[14]+ai3 *b[15];
		}
	}

	Utils3D.matrix4x4MultiplyMFM=function(left,right,out){
		Utils3D.matrix4x4MultiplyFFF(left.elements,right,out.elements);
	}

	Utils3D._tempVector3_0=new Vector3();
	Utils3D._tempVector3_1=new Vector3();
	Utils3D._tempVector3_2=new Vector3();
	Utils3D._tempVector3_3=new Vector3();
	Utils3D._tempVector3_4=new Vector3();
	Utils3D._tempVector3_5=new Vector3();
	Utils3D._tempVector3_6=new Vector3();
	Utils3D._tempArray4_0=new Float32Array(4);
	Utils3D._tempArray16_0=new Float32Array(16);
	Utils3D._tempArray16_1=new Float32Array(16);
	Utils3D._tempArray16_2=new Float32Array(16);
	Utils3D._tempArray16_3=new Float32Array(16);
	__static(Utils3D,
	['_typeToFunO',function(){return this._typeToFunO={"INT16":"writeInt16","SHORT":"writeInt16","UINT16":"writeUint16","UINT32":"writeUint32","FLOAT32":"writeFloat32","INT":"writeInt32","UINT":"writeUint32","BYTE":"writeByte","STRING":"writeUTFString"};}
	]);
	return Utils3D;
})()


/**
*<code>Laya3D</code> 类用于初始化3D设置。
*/
//class Laya3D
var Laya3D=(function(){
	/**
	*创建一个 <code>Laya3D</code> 实例。
	*/
	function Laya3D(){}
	__class(Laya3D,'Laya3D');
	Laya3D._cancelLoadByUrl=function(url){
		Laya.loader.cancelLoadByUrl(url);
		Laya3D._innerFirstLevelLoaderManager.cancelLoadByUrl(url);
		Laya3D._innerSecondLevelLoaderManager.cancelLoadByUrl(url);
		Laya3D._innerThirdLevelLoaderManager.cancelLoadByUrl(url);
		Laya3D._innerFourthLevelLoaderManager.cancelLoadByUrl(url);
	}

	Laya3D._changeWebGLSize=function(width,height){
		WebGL.onStageResize(width,height);
		RenderState.clientWidth=width;
		RenderState.clientHeight=height;
	}

	Laya3D.__init__=function(){
		var createMap=LoaderManager.createMap;
		createMap["lh"]=[Sprite3D,/*CLASS CONST:Laya3D.HIERARCHY*/"SPRITE3DHIERARCHY"];
		createMap["ls"]=[Scene,/*CLASS CONST:Laya3D.HIERARCHY*/"SPRITE3DHIERARCHY"];
		createMap["lmat"]=[StandardMaterial,/*CLASS CONST:Laya3D.MATERIAL*/"MATERIAL"];
		createMap["jpg"]=[Texture2D,"nativeimage"];
		createMap["jpeg"]=[Texture2D,"nativeimage"];
		createMap["png"]=[Texture2D,"nativeimage"];
		createMap["pkm"]=[Texture2D,/*laya.net.Loader.BUFFER*/"arraybuffer"];
		createMap["raw"]=[DataTexture2D,/*laya.net.Loader.BUFFER*/"arraybuffer"];
		createMap["mipmaps"]=[DataTexture2D,/*laya.net.Loader.BUFFER*/"arraybuffer"];
		Loader.parserMap[ /*CLASS CONST:Laya3D.HIERARCHY*/"SPRITE3DHIERARCHY"]=Laya3D._loadHierarchy;
		Loader.parserMap[ /*CLASS CONST:Laya3D.MESH*/"MESH"]=Laya3D._loadMesh;
		Loader.parserMap[ /*CLASS CONST:Laya3D.MATERIAL*/"MATERIAL"]=Laya3D._loadMaterial;
		Loader.parserMap[ /*CLASS CONST:Laya3D.TEXTURECUBE*/"TEXTURECUBE"]=Laya3D._loadTextureCube;
		Loader.parserMap[ /*CLASS CONST:Laya3D.TERRAIN*/"TERRAIN"]=Laya3D._loadTerrain;
		Laya3D._innerFirstLevelLoaderManager.on(/*laya.events.Event.ERROR*/"error",null,Laya3D._eventLoadManagerError);
		Laya3D._innerSecondLevelLoaderManager.on(/*laya.events.Event.ERROR*/"error",null,Laya3D._eventLoadManagerError);
		Laya3D._innerThirdLevelLoaderManager.on(/*laya.events.Event.ERROR*/"error",null,Laya3D._eventLoadManagerError);
		Laya3D._innerFourthLevelLoaderManager.on(/*laya.events.Event.ERROR*/"error",null,Laya3D._eventLoadManagerError);
	}

	Laya3D.READ_BLOCK=function(){
		Laya3D._readData.pos+=4;
		return true;
	}

	Laya3D.READ_DATA=function(){
		Laya3D._DATA.offset=Laya3D._readData.getUint32();
		Laya3D._DATA.size=Laya3D._readData.getUint32();
		return true;
	}

	Laya3D.READ_STRINGS=function(){
		var materialUrls=[];
		var _STRINGS={offset:0,size:0};
		_STRINGS.offset=Laya3D._readData.getUint16();
		_STRINGS.size=Laya3D._readData.getUint16();
		var ofs=Laya3D._readData.pos;
		Laya3D._readData.pos=_STRINGS.offset+Laya3D._DATA.offset;
		for (var i=0;i < _STRINGS.size;i++){
			var string=Laya3D._readData.readUTFString();
			if (string.lastIndexOf(".lmat")!==-1 || string.lastIndexOf(".lpbr")!==-1)
				materialUrls.push(string);
		}
		return materialUrls;
	}

	Laya3D.formatRelativePath=function(base,value){
		var path;
		var char1=value.charAt(0);
		if (char1==="."){
			var parts=(base+value).split("/");
			for (var i=0,len=parts.length;i < len;i++){
				if (parts[i]=='..'){
					var index=i-1;
					if (index > 0 && parts[index]!=='..'){
						parts.splice(index,2);
						i-=2;
					}
				}
			}
			path=parts.join('/');
			}else {
			path=base+value;
		}
		(URL.customFormat !=null)&&(path=URL.customFormat(path,null));
		return path;
	}

	Laya3D._eventLoadManagerError=function(msg){
		Laya.loader.event(/*laya.events.Event.ERROR*/"error",msg);
	}

	Laya3D._addHierarchyInnerUrls=function(urls,urlMap,urlVersion,hierarchyBasePath,path,clas){
		var formatSubUrl=Laya3D.formatRelativePath(hierarchyBasePath,path);
		(urlVersion)&& (formatSubUrl=formatSubUrl+urlVersion);
		urls.push({url:formatSubUrl,clas:clas});
		urlMap[path]=formatSubUrl;
	}

	Laya3D._getSprite3DHierarchyInnerUrls=function(node,firstLevelUrls,secondLevelUrls,fourthLelUrls,urlMap,urlVersion,hierarchyBasePath){
		var i=0,n=0;
		var customProps;
		switch (node.type){
			case "Scene":;
				var lightmaps=node.customProps.lightmaps;
				for (i=0,n=lightmaps.length;i < n;i++){
					var lightMap=lightmaps[i].replace(".exr",".png");
					Laya3D._addHierarchyInnerUrls(fourthLelUrls,urlMap,urlVersion,hierarchyBasePath,lightMap,Texture2D);
				}
				break ;
			case "MeshSprite3D":
			case "TrailSprite3D":
			case "LineSprite3D":
			case "SkinnedMeshSprite3D":;
				var meshPath;
				if (node.instanceParams){
					meshPath=node.instanceParams.loadPath;
					(meshPath)&& (Laya3D._addHierarchyInnerUrls(firstLevelUrls,urlMap,urlVersion,hierarchyBasePath,meshPath,Mesh));
					}else {
					customProps=node.customProps;
					meshPath=customProps.meshPath;
					(meshPath)&& (Laya3D._addHierarchyInnerUrls(firstLevelUrls,urlMap,urlVersion,hierarchyBasePath,meshPath,Mesh));
					var materials=customProps.materials;
					if (materials)
						for (i=0,n=materials.length;i < n;i++){
						var mat=materials[i];
						var clasPaths=mat.type.split('.');
						var clas=Browser.window;
						clasPaths.forEach(function(cls){
							clas=clas[cls];
						});
						if (typeof(clas)=='function')Laya3D._addHierarchyInnerUrls(secondLevelUrls,urlMap,urlVersion,hierarchyBasePath,mat.path,clas);
						else {
							throw('_getSprite3DHierarchyInnerUrls 错误: '+mat.type+' 不是类');
						}
					}
				}
				break ;
			case "ShuriKenParticle3D":
				customProps=node.customProps;
				var parMeshPath=customProps.meshPath;
				(parMeshPath)&& (Laya3D._addHierarchyInnerUrls(firstLevelUrls,urlMap,urlVersion,hierarchyBasePath,parMeshPath,Mesh));
				var materialData=customProps.material;
				if (materialData){
					var clasPaths=materialData.type.split('.');
					var clas=Browser.window;
					clasPaths.forEach(function(cls){
						clas=clas[cls];
					});
					Laya3D._addHierarchyInnerUrls(secondLevelUrls,urlMap,urlVersion,hierarchyBasePath,materialData.path,clas);
					}else {
					var materialPath=customProps.materialPath;
					if (materialPath){
						Laya3D._addHierarchyInnerUrls(secondLevelUrls,urlMap,urlVersion,hierarchyBasePath,materialPath,ShurikenParticleMaterial);
						}else {
						var texturePath=customProps.texturePath;
						if (texturePath)
							Laya3D._addHierarchyInnerUrls(fourthLelUrls,urlMap,urlVersion,hierarchyBasePath,texturePath,Texture2D);
					}
				}
				break ;
			case "Terrain":
				Laya3D._addHierarchyInnerUrls(fourthLelUrls,urlMap,urlVersion,hierarchyBasePath,node.customProps.dataPath,TerrainRes);
				break ;
			};
		var components=node.components;
		for (var k in components){
			var component=components[k];
			switch (k){
				case "Animator":;
					var avatarPath=component.avatarPath;
					if (avatarPath){
						Laya3D._addHierarchyInnerUrls(fourthLelUrls,urlMap,urlVersion,hierarchyBasePath,avatarPath,Avatar);
						}else {
						var avatarData=component.avatar;
						(avatarData)&& (Laya3D._addHierarchyInnerUrls(fourthLelUrls,urlMap,urlVersion,hierarchyBasePath,avatarData.path,Avatar));
					};
					var clipPaths=component.clipPaths;
					for (i=0,n=clipPaths.length;i < n;i++)
					Laya3D._addHierarchyInnerUrls(fourthLelUrls,urlMap,urlVersion,hierarchyBasePath,clipPaths[i],AnimationClip);
					break ;
				}
		};
		var children=node.child;
		for (i=0,n=children.length;i < n;i++)
		Laya3D._getSprite3DHierarchyInnerUrls(children[i],firstLevelUrls,secondLevelUrls,fourthLelUrls,urlMap,urlVersion,hierarchyBasePath);
	}

	Laya3D._loadHierarchy=function(loader){
		loader.on(/*laya.events.Event.LOADED*/"loaded",null,Laya3D._onHierarchylhLoaded,[loader,loader._class._getGroup()]);
		loader.load(loader.url,/*laya.net.Loader.JSON*/"json",false,null,true);
	}

	Laya3D._onHierarchylhLoaded=function(loader,group,lhData){
		if (loader._class.destroyed){
			loader.endLoad();
			}else {
			var url=loader.url;
			var urlVersion=Utils3D.getURLVerion(url);
			var hierarchyBasePath=URL.getPath(url);
			var firstLevUrls=[];
			var secondLevUrls=[];
			var forthLevUrls=[];
			var urlMap={};
			Laya3D._getSprite3DHierarchyInnerUrls(lhData,firstLevUrls,secondLevUrls,forthLevUrls,urlMap,urlVersion,hierarchyBasePath);
			var urlCount=firstLevUrls.length+secondLevUrls.length+forthLevUrls.length;
			var totalProcessCount=urlCount+1;
			var weight=1 / totalProcessCount;
			Laya3D._onProcessChange(loader,0,weight,1.0);
			if (forthLevUrls.length > 0){
				var processCeil=urlCount / totalProcessCount;
				var processHandler=Handler.create(null,Laya3D._onProcessChange,[loader,weight,processCeil],false);
				Laya3D._innerFourthLevelLoaderManager.create(forthLevUrls,Handler.create(null,Laya3D._onHierarchyInnerForthLevResouLoaded,[loader,group,processHandler,lhData,urlMap,firstLevUrls,secondLevUrls,weight+processCeil *forthLevUrls.length,processCeil]),processHandler,null,null,1,true,group);
				}else {
				Laya3D._onHierarchyInnerForthLevResouLoaded(loader,group,null,lhData,urlMap,firstLevUrls,secondLevUrls,weight,processCeil);
			}
		}
	}

	Laya3D._onHierarchyInnerForthLevResouLoaded=function(loader,group,processHandler,lhData,urlMap,firstLevUrls,secondLevUrls,processOffset,processCeil){
		if (loader._class.destroyed){
			loader.endLoad();
			}else {
			(processHandler)&& (processHandler.recover());
			if (secondLevUrls.length > 0){
				var process=Handler.create(null,Laya3D._onProcessChange,[loader,processOffset,processCeil],false);
				Laya3D._innerSecondLevelLoaderManager.create(secondLevUrls,Handler.create(null,Laya3D._onHierarchyInnerSecondLevResouLoaded,[loader,group,process,lhData,urlMap,firstLevUrls,processOffset+processCeil *secondLevUrls.length,processCeil]),processHandler,null,null,1,true,group);
				}else {
				Laya3D._onHierarchyInnerSecondLevResouLoaded(loader,group,null,lhData,urlMap,firstLevUrls,processOffset,processCeil);
			}
		}
	}

	Laya3D._onHierarchyInnerSecondLevResouLoaded=function(loader,group,processHandler,lhData,urlMap,firstLevUrls,processOffset,processCeil){
		if (loader._class.destroyed){
			loader.endLoad();
			}else {
			(processHandler)&& (processHandler.recover());
			if (firstLevUrls.length > 0){
				var process=Handler.create(null,Laya3D._onProcessChange,[loader,processOffset,processCeil],false);
				Laya3D._innerFirstLevelLoaderManager.create(firstLevUrls,Handler.create(null,Laya3D._onHierarchyInnerFirstLevResouLoaded,[loader,process,lhData,urlMap,]),processHandler,null,null,1,true,group);
				}else {
				Laya3D._onHierarchyInnerFirstLevResouLoaded(loader,null,lhData,urlMap);
			}
		}
	}

	Laya3D._onHierarchyInnerFirstLevResouLoaded=function(loader,processHandler,lhData,urlMap){
		(processHandler)&& (processHandler.recover());
		loader.endLoad([lhData,urlMap]);
	}

	Laya3D._loadTerrain=function(loader){
		loader.on(/*laya.events.Event.LOADED*/"loaded",null,Laya3D._onTerrainLtLoaded,[loader,loader._class._getGroup()]);
		loader.load(loader.url,/*laya.net.Loader.JSON*/"json",false,null,true);
	}

	Laya3D._onTerrainLtLoaded=function(loader,group,ltData){
		if (loader._class.destroyed){
			loader.endLoad();
			}else {
			var url=loader.url;
			var urlVersion=Utils3D.getURLVerion(url);
			var terrainBasePath=URL.getPath(url);
			var heightMapURL,textureURLs=[];
			var urlMap={};
			var formatUrl;
			var i=0,n=0,count=0;
			var heightData=ltData.heightData;
			heightMapURL=heightData.url;
			formatUrl=Laya3D.formatRelativePath(terrainBasePath,heightMapURL);
			(urlVersion)&& (formatUrl=formatUrl+urlVersion);
			urlMap[heightMapURL]=formatUrl;
			heightMapURL=formatUrl;
			var detailTextures=ltData.detailTexture;
			for (i=0,n=detailTextures.length;i < n;i++)
			textureURLs.push({url:detailTextures[i].diffuse});
			var normalMaps=ltData.normalMap;
			for (i=0,n=normalMaps.length;i < n;i++)
			textureURLs.push({url:normalMaps[i]});
			var alphaMaps=ltData.alphaMap;
			for (i=0,n=alphaMaps.length;i < n;i++)
			textureURLs.push({url:alphaMaps[i],params:[false,false,/*laya.webgl.WebGLContext.RGBA*/0x1908,true]});
			for (i=0,n=textureURLs.length;i < n;i++){
				var subUrl=textureURLs[i].url;
				formatUrl=Laya3D.formatRelativePath(terrainBasePath,subUrl);
				(urlVersion)&& (formatUrl=formatUrl+urlVersion);
				textureURLs[i].url=formatUrl;
				urlMap[subUrl]=formatUrl;
			};
			var texsUrlCount=textureURLs.length;
			var totalProcessCount=texsUrlCount+2;
			var weight=1 / totalProcessCount;
			Laya3D._onProcessChange(loader,0,weight,1.0);
			var loadInfo={heightMapLoaded:false,texturesLoaded:false};
			var hmProcessHandler=Handler.create(null,Laya3D._onProcessChange,[loader,weight,weight],false);
			Laya3D._innerFourthLevelLoaderManager.create(heightMapURL,Handler.create(null,Laya3D._onTerrainHeightMapLoaded,[loader,hmProcessHandler,ltData,urlMap,loadInfo]),hmProcessHandler,null,[heightData.numX,heightData.numZ,heightData.bitType,heightData.value],1,true,group);
			var texsProcessHandler=Handler.create(null,Laya3D._onProcessChange,[loader,weight *2,texsUrlCount / totalProcessCount],false);
			Laya3D._innerFourthLevelLoaderManager.create(textureURLs,Handler.create(null,Laya3D._onTerrainTexturesLoaded,[loader,texsProcessHandler,ltData,urlMap,loadInfo]),texsProcessHandler,null,null,1,true,group);
		}
	}

	Laya3D._onTerrainHeightMapLoaded=function(loader,processHandler,ltData,urlMap,loadInfo){
		loadInfo.heightMapLoaded=true;
		if (loadInfo.texturesLoaded){
			loader.endLoad([ltData,urlMap]);
			processHandler.recover();
		}
	}

	Laya3D._onTerrainTexturesLoaded=function(loader,processHandler,ltData,urlMap,loadInfo){
		loadInfo.texturesLoaded=true;
		if (loadInfo.heightMapLoaded){
			loader.endLoad([ltData,urlMap]);
			processHandler.recover();
		}
	}

	Laya3D._loadMesh=function(loader){
		loader.on(/*laya.events.Event.LOADED*/"loaded",null,Laya3D._onMeshLmLoaded,[loader,loader._class._getGroup()]);
		loader.load(loader.url,/*laya.net.Loader.BUFFER*/"arraybuffer",false,null,true);
	}

	Laya3D._onMeshLmLoaded=function(loader,group,lmData){
		if (loader._class.destroyed){
			loader.endLoad();
			}else {
			var url=loader.url;
			var urlVersion=Utils3D.getURLVerion(url);
			var meshBasePath=URL.getPath(url);
			var urls;
			var urlMap={};
			var formatSubUrl;
			var i=0,n=0,count=0;
			Laya3D._readData=new Byte(lmData);
			Laya3D._readData.pos=0;
			var version=Laya3D._readData.readUTFString();
			switch (version){
				case "LAYAMODEL:02":
				case "LAYAMODEL:03":
				case "LAYAMODEL:0301":;
					var dataOffset=Laya3D._readData.getUint32();
					Laya3D._readData.pos=Laya3D._readData.pos+4;
					count=Laya3D._readData.getUint16();
					Laya3D._readData.pos=Laya3D._readData.pos+count *8;
					var offset=Laya3D._readData.getUint32();
					count=Laya3D._readData.getUint16();
					Laya3D._readData.pos=dataOffset+offset;
					urls=[];
					for (i=0;i < count;i++){
						var string=Laya3D._readData.readUTFString();
						if (string.lastIndexOf(".lmat")!==-1)
							urls.push(string);
					}
					break ;
				default :
					Laya3D.READ_BLOCK();
					for (i=0;i < 2;i++){
						var index=Laya3D._readData.getUint16();
						var blockName=Laya3D._strings[index];
						var fn=Laya3D["READ_"+blockName];
						if (fn==null)throw new Error("model file err,no this function:"+index+" "+blockName);
						if (i===1)
							urls=fn.call();
						else
						fn.call()
					}
				}
			for (i=0,n=urls.length;i < n;i++){
				var subUrl=urls[i];
				formatSubUrl=Laya3D.formatRelativePath(meshBasePath,subUrl);
				(urlVersion)&& (formatSubUrl=formatSubUrl+urlVersion);
				urls[i]=formatSubUrl;
				urlMap[subUrl]=formatSubUrl;
			}
			if (urls.length > 0){
				var urlCount=1;
				var totalProcessCount=urlCount+1;
				var lmatWeight=1 / totalProcessCount;
				Laya3D._onProcessChange(loader,0,lmatWeight,1.0);
				var processHandler=Handler.create(null,Laya3D._onProcessChange,[loader,lmatWeight,urlCount / totalProcessCount],false);
				Laya3D._innerSecondLevelLoaderManager.create(urls,Handler.create(null,Laya3D._onMeshMateialLoaded,[loader,processHandler,lmData,urlMap]),processHandler,null,null,1,true,group);
				}else {
				loader.endLoad([lmData,urlMap]);
			}
		}
	}

	Laya3D._onMeshMateialLoaded=function(loader,processHandler,lmData,urlMap){
		loader.endLoad([lmData,urlMap]);
		processHandler.recover();
	}

	Laya3D._getMaterialTexturePath=function(path,urlVersion,materialBath){
		var extenIndex=path.length-4;
		if (path.indexOf(".dds")==extenIndex || path.indexOf(".tga")==extenIndex || path.indexOf(".exr")==extenIndex || path.indexOf(".DDS")==extenIndex || path.indexOf(".TGA")==extenIndex || path.indexOf(".EXR")==extenIndex)
			path=path.substr(0,extenIndex)+".png";
		path=Laya3D.formatRelativePath(materialBath,path);
		(urlVersion)&& (path=path+urlVersion);
		return path;
	}

	Laya3D._loadMaterial=function(loader){
		loader.on(/*laya.events.Event.LOADED*/"loaded",null,Laya3D._onMaterilLmatLoaded,[loader,loader._class._getGroup()]);
		loader.load(loader.url,/*laya.net.Loader.JSON*/"json",false,null,true);
	}

	Laya3D._onMaterilLmatLoaded=function(loader,group,lmatData){
		if (loader._class.destroyed){
			loader.endLoad();
			}else {
			var url=loader.url;
			var urlVersion=Utils3D.getURLVerion(url);
			var materialBasePath=URL.getPath(url);
			var urls=[];
			var urlMap={};
			var customProps=lmatData.customProps;
			var formatSubUrl;
			var version=lmatData.version;
			if (version){
				switch (version){
					case "LAYAMATERIAL:01":;
						var textures=lmatData.props.textures;
						for (var i=0,n=textures.length;i < n;i++){
							var tex=textures[i];
							var path=tex.path;
							if (path){
								var extenIndex=path.length-4;
								if (path.indexOf(".exr")==extenIndex || path.indexOf(".EXR")==extenIndex)
									path=path.substr(0,extenIndex)+".png";
								formatSubUrl=Laya3D.formatRelativePath(materialBasePath,path);
								(urlVersion)&& (formatSubUrl=formatSubUrl+urlVersion);
								urls.push({url:formatSubUrl,params:tex.params});
								urlMap[path]=formatSubUrl;
							}
						}
						break ;
					default :
						throw new Error("Laya3D:unkonwn version.");
					}
				}else {
				var diffuseTexture=customProps.diffuseTexture.texture2D;
				if (diffuseTexture){
					formatSubUrl=Laya3D._getMaterialTexturePath(diffuseTexture,urlVersion,materialBasePath);
					urls.push(formatSubUrl);
					urlMap[diffuseTexture]=formatSubUrl;
				}
				if (customProps.normalTexture){
					var normalTexture=customProps.normalTexture.texture2D;
					if (normalTexture){
						formatSubUrl=Laya3D._getMaterialTexturePath(normalTexture,urlVersion,materialBasePath);
						urls.push(formatSubUrl);
						urlMap[normalTexture]=formatSubUrl;
					}
				}
				if (customProps.specularTexture){
					var specularTexture=customProps.specularTexture.texture2D;
					if (specularTexture){
						formatSubUrl=Laya3D._getMaterialTexturePath(specularTexture,urlVersion,materialBasePath);
						urls.push(formatSubUrl);
						urlMap[specularTexture]=formatSubUrl;
					}
				}
				if (customProps.emissiveTexture){
					var emissiveTexture=customProps.emissiveTexture.texture2D;
					if (emissiveTexture){
						formatSubUrl=Laya3D._getMaterialTexturePath(emissiveTexture,urlVersion,materialBasePath);
						urls.push(formatSubUrl);
						urlMap[emissiveTexture]=formatSubUrl;
					}
				}
				if (customProps.ambientTexture){
					var ambientTexture=customProps.ambientTexture.texture2D;
					if (ambientTexture){
						formatSubUrl=Laya3D._getMaterialTexturePath(ambientTexture,urlVersion,materialBasePath);
						urls.push(formatSubUrl);
						urlMap[ambientTexture]=formatSubUrl;
					}
				}
				if (customProps.reflectTexture){
					var reflectTexture=customProps.reflectTexture.texture2D;
					if (reflectTexture){
						formatSubUrl=Laya3D._getMaterialTexturePath(reflectTexture,urlVersion,materialBasePath);
						urls.push(formatSubUrl);
						urlMap[reflectTexture]=formatSubUrl;
					}
				}
			};
			var urlCount=urls.length;
			var totalProcessCount=urlCount+1;
			var lmatWeight=1 / totalProcessCount;
			Laya3D._onProcessChange(loader,0,lmatWeight,1.0);
			if (urlCount > 0){
				var processHandler=Handler.create(null,Laya3D._onProcessChange,[loader,lmatWeight,urlCount / totalProcessCount],false);
				Laya3D._innerFourthLevelLoaderManager.create(urls,Handler.create(null,Laya3D._onMateialTexturesLoaded,[loader,processHandler,lmatData,urlMap]),processHandler,Texture2D,null,1,true,group);
				}else {
				Laya3D._onMateialTexturesLoaded(loader,null,lmatData,null);
			}
		}
	}

	Laya3D._onMateialTexturesLoaded=function(loader,processHandler,lmatData,urlMap){
		loader.endLoad([lmatData,urlMap]);
		(processHandler)&& (processHandler.recover());
	}

	Laya3D._loadTextureCube=function(loader){
		loader.on(/*laya.events.Event.LOADED*/"loaded",null,Laya3D._onTextureCubeLtcLoaded,[loader]);
		loader.load(loader.url,/*laya.net.Loader.JSON*/"json",false,null,true);
	}

	Laya3D._onTextureCubeLtcLoaded=function(loader,ltcData){
		if (loader._class.destroyed){
			loader.endLoad();
			}else {
			var ltcBasePath=URL.getPath(loader.url);
			var urls=[Laya3D.formatRelativePath(ltcBasePath,ltcData.px),Laya3D.formatRelativePath(ltcBasePath,ltcData.nx),Laya3D.formatRelativePath(ltcBasePath,ltcData.py),Laya3D.formatRelativePath(ltcBasePath,ltcData.ny),Laya3D.formatRelativePath(ltcBasePath,ltcData.pz),Laya3D.formatRelativePath(ltcBasePath,ltcData.nz)];
			var ltcWeight=1.0 / 7.0;
			Laya3D._onProcessChange(loader,0,ltcWeight,1.0);
			var processHandler=Handler.create(null,Laya3D._onProcessChange,[loader,ltcWeight,6 / 7],false);
			Laya3D._innerFourthLevelLoaderManager.load(urls,Handler.create(null,Laya3D._onTextureCubeImagesLoaded,[loader,urls,processHandler]),processHandler,"nativeimage");
		}
	}

	Laya3D._onTextureCubeImagesLoaded=function(loader,urls,processHandler){
		var images=[];
		images.length=6;
		for (var i=0;i < 6;i++){
			var url=urls[i];
			images[i]=Loader.getRes(url);
			Loader.clearRes(url);
		}
		loader.endLoad(images);
		processHandler.recover();
	}

	Laya3D._onProcessChange=function(loader,offset,weight,process){
		process=offset+process *weight;
		(process < 1.0)&& (loader.event(/*laya.events.Event.PROGRESS*/"progress",process));
	}

	Laya3D.init=function(width,height,antialias,alpha,premultipliedAlpha,stencil){
		(antialias===void 0)&& (antialias=false);
		(alpha===void 0)&& (alpha=false);
		(premultipliedAlpha===void 0)&& (premultipliedAlpha=true);
		(stencil===void 0)&& (stencil=true);
		RunDriver.update3DLoop=function (){
		}
		RunDriver.cancelLoadByUrl=function (url){
			Laya3D._cancelLoadByUrl(url);
		}
		Config.isAntialias=antialias;
		Config.isAlpha=alpha;
		Config.premultipliedAlpha=premultipliedAlpha;
		Config.isStencil=stencil;
		if (!WebGL.enable())
			return "Laya3D init error,must support webGL!";
			
		RunDriver.changeWebGLSize=Laya3D._changeWebGLSize;
		Render.is3DMode=true;
		Laya.init(width,height);
		Layer.__init__();
		ShaderInit3D.__init__();
		MeshSprite3D.__init__();
		Laya3D.__init__();
		AtlasResourceManager.maxTextureCount=2;
		return null;
	}

	Laya3D.HIERARCHY="SPRITE3DHIERARCHY";
	Laya3D.MESH="MESH";
	Laya3D.MATERIAL="MATERIAL";
	Laya3D.PBRMATERIAL="PBRMTL";
	Laya3D.TEXTURECUBE="TEXTURECUBE";
	Laya3D.TERRAIN="TERRAIN";
	Laya3D._readData=null;
	Laya3D._debugPhasorSprite=null;
	Laya3D.debugMode=false;
	__static(Laya3D,
	['_DATA',function(){return this._DATA={offset:0,size:0};},'_strings',function(){return this._strings=['BLOCK','DATA',"STRINGS"];},'_innerFirstLevelLoaderManager',function(){return this._innerFirstLevelLoaderManager=new LoaderManager();},'_innerSecondLevelLoaderManager',function(){return this._innerSecondLevelLoaderManager=new LoaderManager();},'_innerThirdLevelLoaderManager',function(){return this._innerThirdLevelLoaderManager=new LoaderManager();},'_innerFourthLevelLoaderManager',function(){return this._innerFourthLevelLoaderManager=new LoaderManager();}
	]);
	return Laya3D;
})()


/**
*<code>Transform3D</code> 类用于实现3D变换。
*/
//class laya.d3.animation.AnimationTransform3D extends laya.events.EventDispatcher
var AnimationTransform3D=(function(_super){
	function AnimationTransform3D(owner){
		/**@private */
		//this._localMatrix=null;
		/**@private */
		//this._worldMatrix=null;
		/**@private */
		//this._localPosition=null;
		/**@private */
		//this._localRotation=null;
		/**@private */
		//this._localScale=null;
		/**@private */
		//this._localQuaternionUpdate=false;
		/**@private */
		//this._locaEulerlUpdate=false;
		/**@private */
		//this._localUpdate=false;
		/**@private */
		//this._parent=null;
		/**@private */
		//this._childs=null;
		/**@private */
		//this._localRotationEuler=null;
		/**@private */
		//this._owner=null;
		/**@private */
		//this._worldUpdate=false;
		/**@private */
		//this._entity=null;
		AnimationTransform3D.__super.call(this);
		this._owner=owner;
		this._childs=[];
		this._localMatrix=new Float32Array(16);
		this._localQuaternionUpdate=false;
		this._locaEulerlUpdate=false;
		this._localUpdate=false;
		this._worldUpdate=true;
	}

	__class(AnimationTransform3D,'laya.d3.animation.AnimationTransform3D',_super);
	var __proto=AnimationTransform3D.prototype;
	/**
	*@private
	*/
	__proto._getlocalMatrix=function(){
		if (this._localUpdate){
			Utils3D._createAffineTransformationArray(this._localPosition,this._localRotation,this._localScale,this._localMatrix);
			this._localUpdate=false;
		}
		return this._localMatrix;
	}

	/**
	*@private
	*/
	__proto._onWorldTransform=function(){
		if (!this._worldUpdate){
			this._worldUpdate=true;
			for (var i=0,n=this._childs.length;i < n;i++)
			this._childs[i]._onWorldTransform();
		}
	}

	/**
	*@private
	*/
	__proto._setWorldMatrixAndUpdate=function(matrix){
		this._worldMatrix=matrix;
		if (this._parent==null){
			throw new Error("don't need to set worldMatrix to root Node.");
			}else {
			if (this._parent._parent==null){
				var locMat=this._getlocalMatrix();
				for (var i=0;i < 16;++i)
				this._worldMatrix[i]=locMat[i];
				}else {
				Utils3D.matrix4x4MultiplyFFF(this._parent.getWorldMatrix(),this._getlocalMatrix(),this._worldMatrix);
			}
		}
		this._worldUpdate=false;
	}

	/**
	*@private
	*/
	__proto._setWorldMatrixNoUpdate=function(matrix){
		this._worldMatrix=matrix;
	}

	/**
	*@private
	*/
	__proto._setWorldMatrixIgnoreUpdate=function(matrix){
		this._worldMatrix=matrix;
		this._worldUpdate=false;
	}

	/**
	*获取局部位置。
	*@return 局部位置。
	*/
	__proto.getLocalPosition=function(){
		return this._localPosition;
	}

	/**
	*设置局部位置。
	*@param value 局部位置。
	*/
	__proto.setLocalPosition=function(value){
		if (this._parent){
			this._localPosition=value;
			this._localUpdate=true;
			this._onWorldTransform();
			}else {
			var entityTransform=this._entity.owner._transform;
			var entityPosition=this._entity.localPosition;
			var entityPositionE=entityPosition.elements;
			entityPositionE[0]=value[0];
			entityPositionE[1]=value[1];
			entityPositionE[2]=value[2];
			entityTransform.localPosition=entityPosition;
		}
	}

	/**
	*获取局部旋转。
	*@return 局部旋转。
	*/
	__proto.getLocalRotation=function(){
		if (this._localQuaternionUpdate){
			var eulerE=this._localRotationEuler;
			Utils3D._quaternionCreateFromYawPitchRollArray(eulerE[1] / AnimationTransform3D._angleToRandin,eulerE[0] / AnimationTransform3D._angleToRandin,eulerE[2] / AnimationTransform3D._angleToRandin,this._localRotation);
			this._localQuaternionUpdate=false;
		}
		return this._localRotation;
	}

	/**
	*设置局部旋转。
	*@param value 局部旋转。
	*/
	__proto.setLocalRotation=function(value){
		if (this._parent){
			this._localRotation=value;
			Utils3D.quaterionNormalize(this._localRotation,this._localRotation);
			this._locaEulerlUpdate=true;
			this._localQuaternionUpdate=false;
			this._localUpdate=true;
			this._onWorldTransform();
			}else {
			var entityTransform=this._entity.owner._transform;
			var entityRotation=this._entity.localRotation;
			var entityRotationE=entityRotation.elements;
			entityRotationE[0]=value[0];
			entityRotationE[1]=value[1];
			entityRotationE[2]=value[2];
			entityRotationE[3]=value[3];
			entityTransform.localRotation=entityRotation;
		}
	}

	/**
	*获取局部缩放。
	*@return 局部缩放。
	*/
	__proto.getLocalScale=function(){
		return this._localScale;
	}

	/**
	*设置局部缩放。
	*@param value 局部缩放。
	*/
	__proto.setLocalScale=function(value){
		if (this._parent){
			this._localScale=value;
			this._localUpdate=true;
			this._onWorldTransform();
			}else {
			var entityTransform=this._entity.owner._transform;
			var entityScale=this._entity.localScale;
			var entityScaleE=entityScale.elements;
			entityScaleE[0]=value[0];
			entityScaleE[1]=value[1];
			entityScaleE[2]=value[2];
			entityTransform.localScale=entityScale;
		}
	}

	/**
	*获取局部空间的旋转角度。
	*@return 欧拉角的旋转值，顺序为x、y、z。
	*/
	__proto.getLocalRotationEuler=function(){
		if (this._locaEulerlUpdate){
			Utils3D.getYawPitchRoll(this._localRotation,AnimationTransform3D._tempVector3);
			var eulerE=AnimationTransform3D._tempVector3;
			var localRotationEulerE=this._localRotationEuler;
			localRotationEulerE[0]=eulerE[1] *AnimationTransform3D._angleToRandin;
			localRotationEulerE[1]=eulerE[0] *AnimationTransform3D._angleToRandin;
			localRotationEulerE[2]=eulerE[2] *AnimationTransform3D._angleToRandin;
			this._locaEulerlUpdate=false;
		}
		return this._localRotationEuler;
	}

	/**
	*设置局部空间的旋转角度。
	*@param value 欧拉角的旋转值，顺序为x、y、z。
	*/
	__proto.setLocalRotationEuler=function(value){
		if (this._parent){
			Utils3D._quaternionCreateFromYawPitchRollArray(value[1] / AnimationTransform3D._angleToRandin,value[0] / AnimationTransform3D._angleToRandin,value[2] / AnimationTransform3D._angleToRandin,this._localRotation);
			this._localRotationEuler=value;
			this._locaEulerlUpdate=false;
			this._localQuaternionUpdate=false;
			this._localUpdate=true;
			this._onWorldTransform();
			}else {
			var entityTransform=this._entity.owner._transform;
			var entityLocalRotationEuler=this._entity.localRotationEuler;
			var elements=entityLocalRotationEuler.elements;
			elements[0]=value[0];
			elements[1]=value[1];
			elements[2]=value[2];
			entityTransform.localRotationEuler=entityLocalRotationEuler;
		}
	}

	/**
	*获取世界矩阵。
	*@return 世界矩阵。
	*/
	__proto.getWorldMatrix=function(){
		if (this._worldUpdate){
			if (this._parent._parent !=null){
				Utils3D.matrix4x4MultiplyFFF(this._parent.getWorldMatrix(),this._getlocalMatrix(),this._worldMatrix);
				}else {
				var locMat=this._getlocalMatrix();
				for (var i=0;i < 16;++i)
				this._worldMatrix[i]=locMat[i];
			}
			this._worldUpdate=false;
		}
		return this._worldMatrix;
	}

	/**
	*设置父3D变换。
	*@param value 父3D变换。
	*/
	__proto.setParent=function(value){
		if (this._parent!==value){
			if (this._parent){
				var parentChilds=this._parent._childs;
				var index=parentChilds.indexOf(this);
				parentChilds.splice(index,1);
			}
			if (value){
				value._childs.push(this);
				(value)&& (this._onWorldTransform());
			}
			this._parent=value;
		}
	}

	__static(AnimationTransform3D,
	['_tempVector3',function(){return this._tempVector3=new Float32Array(3);},'_angleToRandin',function(){return this._angleToRandin=180 / Math.PI;}
	]);
	return AnimationTransform3D;
})(EventDispatcher)


/**
*<code>Component3D</code> 类用于创建组件的父类。
*/
//class laya.d3.component.Component3D extends laya.events.EventDispatcher
var Component3D=(function(_super){
	function Component3D(){
		/**@private */
		this._destroyed=false;
		/**@private 唯一标识ID。*/
		this._id=0;
		/**@private 是否启动。*/
		this._enable=false;
		/**@private 所属Sprite3D节点。*/
		this._owner=null;
		/**是否已执行start函数。*/
		this.started=false;
		Component3D.__super.call(this);
		this._destroyed=false;
		this._id=Component3D._uniqueIDCounter;
		Component3D._uniqueIDCounter++;
	}

	__class(Component3D,'laya.d3.component.Component3D',_super);
	var __proto=Component3D.prototype;
	Laya.imps(__proto,{"laya.d3.core.render.IUpdate":true,"laya.resource.IDestroy":true})
	/**
	*@private
	*初始化组件。
	*@param owner 所属Sprite3D节点。
	*/
	__proto._initialize=function(owner){
		this._owner=owner;
		this._enable=true;
		this.started=false;
		this._load(owner);
	}

	/**
	*@private
	*销毁组件。
	*/
	__proto._destroy=function(){
		this._unload(this._owner);
		this._owner=null;
		this._destroyed=true;
	}

	/**
	*@private
	*载入组件时执行,可重写此函数。
	*/
	__proto._load=function(owner){}
	/**
	*@private
	*在任意第一次更新时执行,可重写此函数。
	*/
	__proto._start=function(state){}
	/**
	*@private
	*更新组件,可重写此函数。
	*@param state 渲染状态参数。
	*/
	__proto._update=function(state){}
	/**
	*@private
	*更新的最后阶段执行,可重写此函数。
	*@param state 渲染状态参数。
	*/
	__proto._lateUpdate=function(state){}
	/**
	*@private
	*渲染前设置组件相关参数,可重写此函数。
	*@param state 渲染状态参数。
	*/
	__proto._preRenderUpdate=function(state){}
	/**
	*@private
	*渲染的最后阶段执行,可重写此函数。
	*@param state 渲染状态参数。
	*/
	__proto._postRenderUpdate=function(state){}
	/**
	*@private
	*卸载组件时执行,可重写此函数。
	*/
	__proto._unload=function(owner){
		this.offAll();
	}

	/**
	*@private
	*/
	__proto._cloneTo=function(dest){}
	/**
	*获取唯一标识ID。
	*@return 唯一标识ID。
	*/
	__getset(0,__proto,'id',function(){
		return this._id;
	});

	/**
	*获取是否已销毁。
	*@return 是否已销毁。
	*/
	__getset(0,__proto,'destroyed',function(){
		return this._destroyed;
	});

	/**
	*获取所属Sprite3D节点。
	*@return 所属Sprite3D节点。
	*/
	__getset(0,__proto,'owner',function(){
		return this._owner;
	});

	/**
	*设置是否启用。
	*@param value 是否启动
	*/
	/**
	*获取是否启用。
	*@return 是否启动。
	*/
	__getset(0,__proto,'enable',function(){
		return this._enable;
		},function(value){
		if (this._enable!==value){
			this._enable=value;
			this.event(/*laya.events.Event.ENABLE_CHANGED*/"enablechanged",this._enable);
		}
	});

	/**
	*获取是否为单实例组件。
	*@return 是否为单实例组件。
	*/
	__getset(0,__proto,'isSingleton',function(){
		return Component3D._isSingleton;
	});

	Component3D._isSingleton=true;
	Component3D._uniqueIDCounter=1;
	return Component3D;
})(EventDispatcher)


/**
*<code>GeometryFilter</code> 类用于创建集合体过滤器,抽象类不允许实例。
*/
//class laya.d3.core.GeometryFilter extends laya.events.EventDispatcher
var GeometryFilter=(function(_super){
	function GeometryFilter(){
		/**@private */
		this._destroyed=false;
		GeometryFilter.__super.call(this);
		this._destroyed=false;
	}

	__class(GeometryFilter,'laya.d3.core.GeometryFilter',_super);
	var __proto=GeometryFilter.prototype;
	Laya.imps(__proto,{"laya.resource.IDestroy":true})
	/**
	*@private
	*/
	__proto._destroy=function(){
		this.offAll();
		this._destroyed=true;
	}

	/**@private */
	__getset(0,__proto,'_isAsyncLoaded',function(){
		return true;
	});

	/**
	*@private
	*/
	__getset(0,__proto,'_originalBoundingBoxCorners',function(){
		throw new Error("BaseRender: must override it.");
	});

	/**
	*@private
	*/
	__getset(0,__proto,'_originalBoundingSphere',function(){
		throw new Error("BaseRender: must override it.");
	});

	/**
	*@private
	*/
	__getset(0,__proto,'_originalBoundingBox',function(){
		throw new Error("BaseRender: must override it.");
	});

	/**
	*获取是否已销毁。
	*@return 是否已销毁。
	*/
	__getset(0,__proto,'destroyed',function(){
		return this._destroyed;
	});

	return GeometryFilter;
})(EventDispatcher)


/**
*<code>Render</code> 类用于渲染器的父类，抽象类不允许实例。
*/
//class laya.d3.core.render.BaseRender extends laya.events.EventDispatcher
var BaseRender=(function(_super){
	function BaseRender(owner){
		/**@private */
		//this._id=0;
		/**@private */
		//this._destroyed=false;
		/**@private */
		//this._lightmapScaleOffset=null;
		/**@private */
		//this._lightmapIndex=0;
		/**@private */
		//this._enable=false;
		/**@private */
		//this._receiveShadow=false;
		/**@private */
		//this._materialsInstance=null;
		/**@private */
		//this._boundingSphere=null;
		/**@private */
		//this._boundingBox=null;
		/**@private */
		//this._boundingBoxCenter=null;
		/**@private */
		//this._boundingSphereNeedChange=false;
		/**@private */
		//this._boundingBoxNeedChange=false;
		/**@private */
		//this._boundingBoxCenterNeedChange=false;
		/**@private */
		//this._octreeNodeNeedChange=false;
		/**@private */
		//this._indexInSceneFrustumCullingObjects=0;
		/**@private */
		//this._materials=null;
		/**@private */
		//this._owner=null;
		/**@private */
		//this._renderElements=null;
		/**@private */
		//this._distanceForSort=NaN;
		/**@private */
		//this._treeNode=null;
		/**@private */
		//this._isPartOfStaticBatch=false;
		/**@private */
		//this._staticBatchRootSprite3D=null;
		/**@private */
		//this._staticBatchRenderElements=null;
		/**排序矫正值。*/
		//this.sortingFudge=NaN;
		/**是否产生阴影。 */
		//this.castShadow=false;
		BaseRender.__super.call(this);
		this._id=++BaseRender._uniqueIDCounter;
		this._indexInSceneFrustumCullingObjects=-1;
		this._boundingBox=new BoundBox(new Vector3(),new Vector3());
		this._boundingBoxCenter=new Vector3();
		this._boundingSphere=new BoundSphere(new Vector3(),0);
		this._boundingSphereNeedChange=true;
		this._boundingBoxNeedChange=true;
		this._boundingBoxCenterNeedChange=true;
		this._octreeNodeNeedChange=true;
		this._materials=[];
		this._renderElements=[];
		this._isPartOfStaticBatch=false;
		this._destroyed=false;
		this._owner=owner;
		this._enable=true;
		this._materialsInstance=[];
		this.lightmapIndex=-1;
		this.castShadow=false;
		this.sortingFudge=0.0;
		this._owner.transform.on(/*laya.events.Event.WORLDMATRIX_NEEDCHANGE*/"worldmatrixneedchanged",this,this._onWorldMatNeedChange);
	}

	__class(BaseRender,'laya.d3.core.render.BaseRender',_super);
	var __proto=BaseRender.prototype;
	Laya.imps(__proto,{"laya.resource.IDestroy":true})
	/**
	*@private
	*/
	__proto._changeMaterialReference=function(lastValue,value){
		(lastValue)&& (lastValue._removeReference());
		value._addReference();
	}

	/**
	*@private
	*/
	__proto._getInstanceMaterial=function(material,index){
		var insMat=/*__JS__ */new material.constructor();
		material.cloneTo(insMat);
		insMat.name=insMat.name+"(Instance)";
		this._materialsInstance[index]=true;
		this._changeMaterialReference(this._materials[index],insMat);
		this._materials[index]=insMat;
		return insMat;
	}

	/**
	*@private
	*/
	__proto._setShaderValuelightMap=function(lightMap){
		this._setShaderValueTexture(/*laya.d3.core.RenderableSprite3D.LIGHTMAP*/3,lightMap);
	}

	/**
	*@private
	*/
	__proto._onWorldMatNeedChange=function(){
		this._boundingSphereNeedChange=true;
		this._boundingBoxNeedChange=true;
		this._boundingBoxCenterNeedChange=true;
		this._octreeNodeNeedChange=true;
	}

	/**
	*@private
	*/
	__proto._renderRenderableBoundBox=function(){
		var linePhasor=Laya3D._debugPhasorSprite;
		var boundBox=this.boundingBox;
		var corners=BaseRender._tempBoundBoxCorners;
		boundBox.getCorners(corners);
		linePhasor.line(corners[0],BaseRender._greenColor,corners[1],BaseRender._greenColor);
		linePhasor.line(corners[2],BaseRender._greenColor,corners[3],BaseRender._greenColor);
		linePhasor.line(corners[4],BaseRender._greenColor,corners[5],BaseRender._greenColor);
		linePhasor.line(corners[6],BaseRender._greenColor,corners[7],BaseRender._greenColor);
		linePhasor.line(corners[0],BaseRender._greenColor,corners[3],BaseRender._greenColor);
		linePhasor.line(corners[1],BaseRender._greenColor,corners[2],BaseRender._greenColor);
		linePhasor.line(corners[2],BaseRender._greenColor,corners[6],BaseRender._greenColor);
		linePhasor.line(corners[3],BaseRender._greenColor,corners[7],BaseRender._greenColor);
		linePhasor.line(corners[0],BaseRender._greenColor,corners[4],BaseRender._greenColor);
		linePhasor.line(corners[1],BaseRender._greenColor,corners[5],BaseRender._greenColor);
		linePhasor.line(corners[4],BaseRender._greenColor,corners[7],BaseRender._greenColor);
		linePhasor.line(corners[5],BaseRender._greenColor,corners[6],BaseRender._greenColor);
	}

	/**
	*@private
	*/
	__proto._calculateBoundingSphere=function(){
		throw("BaseRender: must override it.");
	}

	/**
	*@private
	*/
	__proto._calculateBoundingBox=function(){
		throw("BaseRender: must override it.");
	}

	/**
	*@private
	*/
	__proto._setShaderValueTexture=function(shaderName,texture){
		this._owner._shaderValues.setValue(shaderName,texture);
	}

	/**
	*@private
	*/
	__proto._setShaderValueMatrix4x4=function(shaderName,matrix4x4){
		this._owner._shaderValues.setValue(shaderName,matrix4x4 ? matrix4x4.elements :null);
	}

	/**
	*设置颜色。
	*@param shaderIndex shader索引。
	*@param color 颜色向量。
	*/
	__proto._setShaderValueColor=function(shaderIndex,color){
		this._owner._shaderValues.setValue(shaderIndex,color ? color.elements :null);
	}

	/**
	*设置Buffer。
	*@param shaderIndex shader索引。
	*@param buffer buffer数据。
	*/
	__proto._setShaderValueBuffer=function(shaderIndex,buffer){
		this._owner._shaderValues.setValue(shaderIndex,buffer);
	}

	/**
	*设置整型。
	*@param shaderIndex shader索引。
	*@param i 整形。
	*/
	__proto._setShaderValueInt=function(shaderIndex,i){
		this._owner._shaderValues.setValue(shaderIndex,i);
	}

	/**
	*设置布尔。
	*@param shaderIndex shader索引。
	*@param b 布尔。
	*/
	__proto._setShaderValueBool=function(shaderIndex,b){
		this._owner._shaderValues.setValue(shaderIndex,b);
	}

	/**
	*设置浮点。
	*@param shaderIndex shader索引。
	*@param i 浮点。
	*/
	__proto._setShaderValueNumber=function(shaderIndex,number){
		this._owner._shaderValues.setValue(shaderIndex,number);
	}

	/**
	*设置二维向量。
	*@param shaderIndex shader索引。
	*@param vector2 二维向量。
	*/
	__proto._setShaderValueVector2=function(shaderIndex,vector2){
		this._owner._shaderValues.setValue(shaderIndex,vector2 ? vector2.elements :null);
	}

	/**
	*增加Shader宏定义。
	*@param value 宏定义。
	*/
	__proto._addShaderDefine=function(value){
		this._owner._shaderDefineValue |=value;
	}

	/**
	*移除Shader宏定义。
	*@param value 宏定义。
	*/
	__proto._removeShaderDefine=function(value){
		this._owner._shaderDefineValue &=~value;
	}

	/**
	*@private
	*/
	__proto._renderUpdate=function(projectionView){
		return true;
	}

	/**
	*@private
	*/
	__proto._applyLightMapParams=function(){
		if (this._lightmapIndex >=0){
			var scene=this._owner.scene;
			if (scene){
				var lightMaps=scene.getlightmaps();
				var lightMap=lightMaps[this._lightmapIndex];
				if (lightMap){
					this._addShaderDefine(RenderableSprite3D.SAHDERDEFINE_LIGHTMAP);
					if (lightMap.loaded)
						this._setShaderValuelightMap(lightMap);
					else
					lightMap.once(/*laya.events.Event.LOADED*/"loaded",this,this._setShaderValuelightMap);
					}else {
					this._removeShaderDefine(RenderableSprite3D.SAHDERDEFINE_LIGHTMAP);
				}
				}else {
				this._removeShaderDefine(RenderableSprite3D.SAHDERDEFINE_LIGHTMAP);
			}
			}else {
			this._removeShaderDefine(RenderableSprite3D.SAHDERDEFINE_LIGHTMAP);
		}
	}

	/**
	*@private
	*/
	__proto._updateOctreeNode=function(){
		var treeNode=this._treeNode;
		if (treeNode && this._octreeNodeNeedChange){
			treeNode.updateObject(this);
			this._octreeNodeNeedChange=false;
		}
	}

	/**
	*@private
	*/
	__proto._destroy=function(){
		this.offAll();
		var i=0,n=0;
		for (i=0,n=this._renderElements.length;i < n;i++)
		this._renderElements[i]._destroy();
		for (i=0,n=this._materials.length;i < n;i++)
		this._materials[i]._removeReference();
		this._renderElements=null;
		this._owner=null;
		this._materials=null;
		this._boundingBox=null;
		this._boundingBoxCenter=null;
		this._boundingSphere=null;
		this._lightmapScaleOffset=null;
		this._destroyed=true;
	}

	/**
	*获取包围球,只读,不允许修改其值。
	*@return 包围球。
	*/
	__getset(0,__proto,'boundingSphere',function(){
		if (this._boundingSphereNeedChange){
			this._calculateBoundingSphere();
			this._boundingSphereNeedChange=false;
		}
		return this._boundingSphere;
	});

	/**
	*获取唯一标识ID,通常用于识别。
	*/
	__getset(0,__proto,'id',function(){
		return this._id;
	});

	/**
	*设置第一个实例材质。
	*@param value 第一个实例材质。
	*/
	/**
	*返回第一个实例材质,第一次使用会拷贝实例对象。
	*@return 第一个实例材质。
	*/
	__getset(0,__proto,'material',function(){
		var material=this._materials[0];
		if (material && !this._materialsInstance[0]){
			var insMat=this._getInstanceMaterial(material,0);
			this.event(/*laya.events.Event.MATERIAL_CHANGED*/"materialchanged",[this,0,insMat]);
		}
		return this._materials[0];
		},function(value){
		this.sharedMaterial=value;
	});

	/**
	*设置第一个材质。
	*@param value 第一个材质。
	*/
	/**
	*返回第一个材质。
	*@return 第一个材质。
	*/
	__getset(0,__proto,'sharedMaterial',function(){
		return this._materials[0];
		},function(value){
		var lastValue=this._materials[0];
		if (lastValue!==value){
			this._materials[0]=value;
			this._materialsInstance[0]=false;
			this._changeMaterialReference(lastValue,value);
			this.event(/*laya.events.Event.MATERIAL_CHANGED*/"materialchanged",[this,0,value]);
		}
	});

	/**
	*设置光照贴图的索引。
	*@param value 光照贴图的索引。
	*/
	/**
	*获取光照贴图的索引。
	*@return 光照贴图的索引。
	*/
	__getset(0,__proto,'lightmapIndex',function(){
		return this._lightmapIndex;
		},function(value){
		this._lightmapIndex=value;
		this._applyLightMapParams();
	});

	/**
	*设置光照贴图的缩放和偏移。
	*@param 光照贴图的缩放和偏移。
	*/
	/**
	*获取光照贴图的缩放和偏移。
	*@return 光照贴图的缩放和偏移。
	*/
	__getset(0,__proto,'lightmapScaleOffset',function(){
		return this._lightmapScaleOffset;
		},function(value){
		this._lightmapScaleOffset=value;
		this._setShaderValueColor(/*laya.d3.core.RenderableSprite3D.LIGHTMAPSCALEOFFSET*/2,value);
		this._addShaderDefine(RenderableSprite3D.SHADERDEFINE_SCALEOFFSETLIGHTINGMAPUV);
	});

	/**
	*设置是否可用。
	*@param value 是否可用。
	*/
	/**
	*获取是否可用。
	*@return 是否可用。
	*/
	__getset(0,__proto,'enable',function(){
		return this._enable;
		},function(value){
		this._enable=value;
		this.event(/*laya.events.Event.ENABLE_CHANGED*/"enablechanged",[this,value]);
	});

	/**
	*设置实例材质列表。
	*@param value 实例材质列表。
	*/
	/**
	*获取潜拷贝实例材质列表,第一次使用会拷贝实例对象。
	*@return 浅拷贝实例材质列表。
	*/
	__getset(0,__proto,'materials',function(){
		for (var i=0,n=this._materials.length;i < n;i++){
			if (!this._materialsInstance[i]){
				var insMat=this._getInstanceMaterial(this._materials[i],i);
				this.event(/*laya.events.Event.MATERIAL_CHANGED*/"materialchanged",[this,i,insMat]);
			}
		}
		return this._materials.slice();
		},function(value){
		this.sharedMaterials=value;
	});

	/**
	*设置材质列表。
	*@param value 材质列表。
	*/
	/**
	*获取浅拷贝材质列表。
	*@return 浅拷贝材质列表。
	*/
	__getset(0,__proto,'sharedMaterials',function(){
		var materials=this._materials.slice();
		return materials;
		},function(value){
		if (!value)
			throw new Error("MeshRender: shadredMaterials value can't be null.");
		var len=value.length;
		this._materialsInstance.length=len;
		for (var i=0;i < len;i++){
			var lastValue=this._materials[i];
			if (lastValue!==value[i]){
				this._materialsInstance[i]=false;
				this._changeMaterialReference(lastValue,value[i]);
				this.event(/*laya.events.Event.MATERIAL_CHANGED*/"materialchanged",[this,i,value[i]]);
			}
		}
		this._materials=value;
	});

	/**
	*获取包围盒,只读,不允许修改其值。
	*@return 包围盒。
	*/
	__getset(0,__proto,'boundingBox',function(){
		if (this._boundingBoxNeedChange){
			this._calculateBoundingBox();
			this._boundingBoxNeedChange=false;
		}
		return this._boundingBox;
	});

	/**
	*获取包围盒中心,不允许修改其值。
	*@return 包围盒中心。
	*/
	__getset(0,__proto,'boundingBoxCenter',function(){
		if (this._boundingBoxCenterNeedChange){
			var boundBox=this.boundingBox;
			Vector3.add(boundBox.min,boundBox.max,this._boundingBoxCenter);
			Vector3.scale(this._boundingBoxCenter,0.5,this._boundingBoxCenter);
			this._boundingBoxCenterNeedChange=false;
		}
		return this._boundingBoxCenter;
	});

	/**
	*获取是否已销毁。
	*@return 是否已销毁。
	*/
	__getset(0,__proto,'destroyed',function(){
		return this._destroyed;
	});

	BaseRender._uniqueIDCounter=0;
	__static(BaseRender,
	['_tempBoundBoxCorners',function(){return this._tempBoundBoxCorners=/*new vector.<>*/[new Vector3(),new Vector3(),new Vector3(),new Vector3(),new Vector3(),new Vector3(),new Vector3(),new Vector3()];},'_greenColor',function(){return this._greenColor=new Vector4(0.0,1.0,0.0,1.0);}
	]);
	return BaseRender;
})(EventDispatcher)


/**
*<code>Transform3D</code> 类用于实现3D变换。
*/
//class laya.d3.core.Transform3D extends laya.events.EventDispatcher
var Transform3D=(function(_super){
	function Transform3D(owner){
		/**@private */
		this._owner=null;
		/**@private */
		this._localQuaternionUpdate=false;
		/**@private */
		this._locaEulerlUpdate=false;
		/**@private */
		this._localUpdate=false;
		/**@private */
		this._worldUpdate=true;
		/**@private */
		this._positionUpdate=true;
		/**@private */
		this._rotationUpdate=true;
		/**@private */
		this._scaleUpdate=true;
		/**@private */
		this._parent=null;
		/**@private */
		this._childs=null;
		/**@private */
		this._dummy=null;
		/**变换中心点,注意:该中心点不受变换的影响。*/
		this.pivot=null;
		Transform3D.__super.call(this);
		this._localPosition=new Vector3();
		this._localRotation=new Quaternion(0,0,0,1);
		this._localScale=new Vector3(1,1,1);
		this._localRotationEuler=new Vector3();
		this._localMatrix=new Matrix4x4();
		this._position=new Vector3();
		this._rotation=new Quaternion(0,0,0,1);
		this._scale=new Vector3(1,1,1);
		this._worldMatrix=new Matrix4x4();
		this._forward=new Vector3();
		this._up=new Vector3();
		this._right=new Vector3();
		this._owner=owner;
		this._childs=[];
	}

	__class(Transform3D,'laya.d3.core.Transform3D',_super);
	var __proto=Transform3D.prototype;
	/**
	*@private
	*/
	__proto._updateLocalMatrix=function(){
		if (this.pivot && (this.pivot.x!==0 || this.pivot.y!==0 || this.pivot.z!==0)){
			var scalePivot=Transform3D._tempVector30;
			Vector3.multiply(this.pivot,this._localScale,scalePivot);
			var scaleOffsetPosition=Transform3D._tempVector31;
			Vector3.subtract(scalePivot,this.pivot,scaleOffsetPosition);
			var rotationOffsetPosition=Transform3D._tempVector32;
			var localRot=this.localRotation;
			Vector3.transformQuat(scalePivot,localRot,rotationOffsetPosition);
			Vector3.subtract(rotationOffsetPosition,scalePivot,rotationOffsetPosition);
			var resultLocalPosition=Transform3D._tempVector33;
			Vector3.subtract(this._localPosition,scaleOffsetPosition,resultLocalPosition);
			Vector3.subtract(resultLocalPosition,rotationOffsetPosition,resultLocalPosition);
			Matrix4x4.createAffineTransformation(resultLocalPosition,localRot,this._localScale,this._localMatrix);
			}else {
			Matrix4x4.createAffineTransformation(this._localPosition,this.localRotation,this._localScale,this._localMatrix);
		}
	}

	/**
	*@private
	*/
	__proto._onWorldPositionRotationTransform=function(){
		if (!this._worldUpdate || !this._positionUpdate || !this._rotationUpdate){
			this._worldUpdate=this._positionUpdate=this._rotationUpdate=true;
			this.event(/*laya.events.Event.WORLDMATRIX_NEEDCHANGE*/"worldmatrixneedchanged");
			for (var i=0,n=this._childs.length;i < n;i++)
			this._childs[i]._onWorldPositionRotationTransform();
		}
	}

	/**
	*@private
	*/
	__proto._onWorldPositionScaleTransform=function(){
		if (!this._worldUpdate || !this._positionUpdate || !this._scaleUpdate){
			this._worldUpdate=this._positionUpdate=this._scaleUpdate=true;
			this.event(/*laya.events.Event.WORLDMATRIX_NEEDCHANGE*/"worldmatrixneedchanged");
			for (var i=0,n=this._childs.length;i < n;i++)
			this._childs[i]._onWorldPositionScaleTransform();
		}
	}

	/**
	*@private
	*/
	__proto._onWorldPositionTransform=function(){
		if (!this._worldUpdate || !this._positionUpdate){
			this._worldUpdate=this._positionUpdate=true;
			this.event(/*laya.events.Event.WORLDMATRIX_NEEDCHANGE*/"worldmatrixneedchanged");
			for (var i=0,n=this._childs.length;i < n;i++)
			this._childs[i]._onWorldPositionTransform();
		}
	}

	/**
	*@private
	*/
	__proto._onWorldRotationTransform=function(){
		if (!this._worldUpdate || !this._rotationUpdate){
			this._worldUpdate=this._rotationUpdate=true;
			this.event(/*laya.events.Event.WORLDMATRIX_NEEDCHANGE*/"worldmatrixneedchanged");
			for (var i=0,n=this._childs.length;i < n;i++)
			this._childs[i]._onWorldPositionRotationTransform();
		}
	}

	/**
	*@private
	*/
	__proto._onWorldScaleTransform=function(){
		if (!this._worldUpdate || !this._scaleUpdate){
			this._worldUpdate=this._scaleUpdate=true;
			this.event(/*laya.events.Event.WORLDMATRIX_NEEDCHANGE*/"worldmatrixneedchanged");
			for (var i=0,n=this._childs.length;i < n;i++)
			this._childs[i]._onWorldPositionScaleTransform();
		}
	}

	/**
	*@private
	*/
	__proto._onWorldTransform=function(){
		if (!this._worldUpdate || !this._positionUpdate || !this._rotationUpdate || !this._scaleUpdate){
			this._worldUpdate=this._positionUpdate=this._rotationUpdate=this._scaleUpdate=true;
			this.event(/*laya.events.Event.WORLDMATRIX_NEEDCHANGE*/"worldmatrixneedchanged");
			for (var i=0,n=this._childs.length;i < n;i++)
			this._childs[i]._onWorldTransform();
		}
	}

	/**
	*平移变换。
	*@param translation 移动距离。
	*@param isLocal 是否局部空间。
	*/
	__proto.translate=function(translation,isLocal){
		(isLocal===void 0)&& (isLocal=true);
		if (isLocal){
			Matrix4x4.createFromQuaternion(this.localRotation,Transform3D._tempMatrix0);
			Vector3.transformCoordinate(translation,Transform3D._tempMatrix0,Transform3D._tempVector30);
			Vector3.add(this.localPosition,Transform3D._tempVector30,this._localPosition);
			this.localPosition=this._localPosition;
			}else {
			Vector3.add(this.position,translation,this._position);
			this.position=this._position;
		}
	}

	/**
	*旋转变换。
	*@param rotations 旋转幅度。
	*@param isLocal 是否局部空间。
	*@param isRadian 是否弧度制。
	*/
	__proto.rotate=function(rotation,isLocal,isRadian){
		(isLocal===void 0)&& (isLocal=true);
		(isRadian===void 0)&& (isRadian=true);
		var rot;
		if (isRadian){
			rot=rotation;
			}else {
			Vector3.scale(rotation,Math.PI / 180.0,Transform3D._tempVector30);
			rot=Transform3D._tempVector30;
		}
		Quaternion.createFromYawPitchRoll(rot.y,rot.x,rot.z,Transform3D._tempQuaternion0);
		if (isLocal){
			Quaternion.multiply(this._localRotation,Transform3D._tempQuaternion0,this._localRotation);
			this.localRotation=this._localRotation;
			}else {
			Quaternion.multiply(Transform3D._tempQuaternion0,this.rotation,this._rotation);
			this.rotation=this._rotation;
		}
	}

	/**
	*观察目标位置。
	*@param target 观察目标。
	*@param up 向上向量。
	*@param isLocal 是否局部空间。
	*/
	__proto.lookAt=function(target,up,isLocal){
		(isLocal===void 0)&& (isLocal=false);
		var targetE=target.elements;
		var eyeE;
		if (isLocal){
			eyeE=this._localPosition.elements;
			if (Math.abs(eyeE[0]-targetE[0])< MathUtils3D.zeroTolerance && Math.abs(eyeE[1]-targetE[1])< MathUtils3D.zeroTolerance && Math.abs(eyeE[2]-targetE[2])< MathUtils3D.zeroTolerance)
				return;
			Quaternion.lookAt(this._localPosition,target,up,this._localRotation);
			this._localRotation.invert(this._localRotation);
			this.localRotation=this._localRotation;
			}else {
			var worldPosition=this.position;
			eyeE=worldPosition.elements;
			if (Math.abs(eyeE[0]-targetE[0])< MathUtils3D.zeroTolerance && Math.abs(eyeE[1]-targetE[1])< MathUtils3D.zeroTolerance && Math.abs(eyeE[2]-targetE[2])< MathUtils3D.zeroTolerance)
				return;
			Quaternion.lookAt(worldPosition,target,up,this._rotation);
			this._rotation.invert(this._rotation);
			this.rotation=this._rotation;
		}
	}

	/**
	*@private
	*/
	__getset(0,__proto,'_isFrontFaceInvert',function(){
		var scale=this.scale;
		var isInvert=scale.x < 0;
		(scale.y < 0)&& (isInvert=!isInvert);
		(scale.z < 0)&& (isInvert=!isInvert);
		return isInvert;
	});

	/**
	*获取所属精灵。
	*/
	__getset(0,__proto,'owner',function(){
		return this._owner;
	});

	/**
	*设置局部旋转。
	*@param value 局部旋转。
	*/
	/**
	*获取局部旋转。
	*@return 局部旋转。
	*/
	__getset(0,__proto,'localRotation',function(){
		if (this._localQuaternionUpdate){
			var eulerE=this._localRotationEuler.elements;
			Quaternion.createFromYawPitchRoll(eulerE[1] / Transform3D._angleToRandin,eulerE[0] / Transform3D._angleToRandin,eulerE[2] / Transform3D._angleToRandin,this._localRotation);
		}
		return this._localRotation;
		},function(value){
		this._localRotation=value;
		this._localRotation.normalize(this._localRotation);
		this._locaEulerlUpdate=true;
		this._localQuaternionUpdate=false;
		this._localUpdate=true;
		if (this.pivot && (this.pivot.x!==0 || this.pivot.y!==0 || this.pivot.z!==0))
			this._onWorldPositionRotationTransform();
		else
		this._onWorldRotationTransform();
	});

	/**
	*设置世界矩阵。
	*@param value 世界矩阵。
	*/
	/**
	*获取世界矩阵。
	*@return 世界矩阵。
	*/
	__getset(0,__proto,'worldMatrix',function(){
		if (this._worldUpdate){
			if (this._parent !=null)
				Matrix4x4.multiply(this._parent.worldMatrix,this.localMatrix,this._worldMatrix);
			else
			this.localMatrix.cloneTo(this._worldMatrix);
			this._worldUpdate=false;
		}
		return this._worldMatrix;
		},function(value){
		if (this._parent===null){
			value.cloneTo(this._localMatrix);
			}else {
			this._parent.worldMatrix.invert(this._localMatrix);
			Matrix4x4.multiply(this._localMatrix,value,this._localMatrix);
		}
		this.localMatrix=this._localMatrix;
		this._worldMatrix=value;
		this._worldUpdate=false;
	});

	/**
	*获取世界矩阵是否需要更新。
	*@return 世界矩阵是否需要更新。
	*/
	__getset(0,__proto,'worldNeedUpdate',function(){
		return this._worldUpdate;
	});

	/**
	*设置局部矩阵。
	*@param value 局部矩阵。
	*/
	/**
	*获取局部矩阵。
	*@return 局部矩阵。
	*/
	__getset(0,__proto,'localMatrix',function(){
		if (this._localUpdate){
			this._updateLocalMatrix();
			this._localUpdate=false;
		}
		return this._localMatrix;
		},function(value){
		this._localMatrix=value;
		this._localMatrix.decomposeTransRotScale(this._localPosition,this._localRotation,this._localScale);
		this._localUpdate=false;
		this._onWorldTransform();
	});

	/**
	*设置关联虚拟变换。
	*@param value 虚拟变换。
	*/
	/**
	*获取关联虚拟变换。
	*@return 虚拟变换。
	*/
	__getset(0,__proto,'dummy',function(){
		return this._dummy;
		},function(value){
		if (this._dummy!==value){
			(this._dummy)&& (this._dummy._entity=null);
			(value)&& (value._entity=this);
			this._dummy=value;
		}
	});

	/**
	*设置局部位置。
	*@param value 局部位置。
	*/
	/**
	*获取局部位置。
	*@return 局部位置。
	*/
	__getset(0,__proto,'localPosition',function(){
		return this._localPosition;
		},function(value){
		this._localPosition=value;
		this._localUpdate=true;
		this._onWorldPositionTransform();
	});

	/**
	*设置世界位置。
	*@param value 世界位置。
	*/
	/**
	*获取世界位置。
	*@return 世界位置。
	*/
	__getset(0,__proto,'position',function(){
		if (this._positionUpdate){
			if (this._parent !=null){
				var parentPosition=this._parent.position;
				Vector3.multiply(this._localPosition,this._parent.scale,Transform3D._tempVector30);
				Vector3.transformQuat(Transform3D._tempVector30,this._parent.rotation,Transform3D._tempVector30);
				Vector3.add(parentPosition,Transform3D._tempVector30,this._position);
				}else {
				this._localPosition.cloneTo(this._position);
			}
			this._positionUpdate=false;
		}
		return this._position;
		},function(value){
		if (this._parent !=null){
			Vector3.subtract(value,this._parent.position,this._localPosition);
			var parentScaleE=this._parent.scale.elements;
			var psX=parentScaleE[0],psY=parentScaleE[1],psZ=parentScaleE[2];
			if (psX!==1.0 || psY!==1.0 || psZ!==1.0){
				var invertScale=Transform3D._tempVector30;
				var invertScaleE=invertScale.elements;
				invertScaleE[0]=1.0 / psX;
				invertScaleE[1]=1.0 / psY;
				invertScaleE[2]=1.0 / psZ;
				Vector3.multiply(this._localPosition,invertScale,this._localPosition);
			};
			var parentRotation=this._parent.rotation;
			parentRotation.invert(Transform3D._tempQuaternion0);
			Vector3.transformQuat(this._localPosition,Transform3D._tempQuaternion0,this._localPosition);
			}else {
			value.cloneTo(this._localPosition);
		}
		this.localPosition=this._localPosition;
		this._position=value;
		this._positionUpdate=false;
	});

	/**
	*设置局部缩放。
	*@param value 局部缩放。
	*/
	/**
	*获取局部缩放。
	*@return 局部缩放。
	*/
	__getset(0,__proto,'localScale',function(){
		return this._localScale;
		},function(value){
		this._localScale=value;
		this._localUpdate=true;
		if (this.pivot && (this.pivot.x!==0 || this.pivot.y!==0 || this.pivot.z!==0))
			this._onWorldPositionScaleTransform();
		else
		this._onWorldScaleTransform();
	});

	/**
	*设置局部空间的旋转角度。
	*@param value 欧拉角的旋转值，顺序为x、y、z。
	*/
	/**
	*获取局部空间的旋转角度。
	*@return 欧拉角的旋转值，顺序为x、y、z。
	*/
	__getset(0,__proto,'localRotationEuler',function(){
		if (this._locaEulerlUpdate){
			this._localRotation.getYawPitchRoll(Transform3D._tempVector30);
			var eulerE=Transform3D._tempVector30.elements;
			var localRotationEulerE=this._localRotationEuler.elements;
			localRotationEulerE[0]=eulerE[1] *Transform3D._angleToRandin;
			localRotationEulerE[1]=eulerE[0] *Transform3D._angleToRandin;
			localRotationEulerE[2]=eulerE[2] *Transform3D._angleToRandin;
		}
		return this._localRotationEuler;
		},function(value){
		this._localRotationEuler=value;
		this._locaEulerlUpdate=false;
		this._localQuaternionUpdate=true;
		this._localUpdate=true;
		if (this.pivot && (this.pivot.x!==0 || this.pivot.y!==0 || this.pivot.z!==0))
			this._onWorldPositionRotationTransform();
		else
		this._onWorldRotationTransform();
	});

	/**
	*设置世界旋转。
	*@param value 世界旋转。
	*/
	/**
	*获取世界旋转。
	*@return 世界旋转。
	*/
	__getset(0,__proto,'rotation',function(){
		if (this._rotationUpdate){
			if (this._parent !=null)
				Quaternion.multiply(this._parent.rotation,this.localRotation,this._rotation);
			else
			this.localRotation.cloneTo(this._rotation);
			this._rotationUpdate=false;
		}
		return this._rotation;
		},function(value){
		if (this._parent !=null){
			this._parent.rotation.invert(Transform3D._tempQuaternion0);
			Quaternion.multiply(value,Transform3D._tempQuaternion0,this._localRotation);
			}else {
			value.cloneTo(this._localRotation);
		}
		this.localRotation=this._localRotation;
		this._rotation=value;
		this._rotationUpdate=false;
	});

	/**
	*设置世界缩放。
	*@param value 世界缩放。
	*/
	/**
	*获取世界缩放。
	*@return 世界缩放。
	*/
	__getset(0,__proto,'scale',function(){
		if (!this._scaleUpdate)
			return this._scale;
		if (this._parent!==null)
			Vector3.multiply(this._parent.scale,this._localScale,this._scale);
		else
		this._localScale.cloneTo(this._scale);
		this._scaleUpdate=false;
		return this._scale;
		},function(value){
		if (this._parent!==null){
			var pScaleE=this._parent.scale.elements;
			var invPScaleE=Transform3D._tempVector30.elements;
			invPScaleE[0]=1.0 / pScaleE[0];
			invPScaleE[1]=1.0 / pScaleE[1];
			invPScaleE[2]=1.0 / pScaleE[2];
			Vector3.multiply(value,Transform3D._tempVector30,this._localScale);
			}else {
			value.cloneTo(this._localScale);
		}
		this.localScale=this._localScale;
		this._scale=value;
		this._scaleUpdate=false;
	});

	/**
	*设置局部空间的旋转角度。
	*@param 欧拉角的旋转值，顺序为x、y、z。
	*/
	__getset(0,__proto,'rotationEuler',null,function(value){
		Quaternion.createFromYawPitchRoll(value.y,value.x,value.z,this._rotation);
		this.rotation=this._rotation;
	});

	/**
	*获取向前方向。
	*@return 向前方向。
	*/
	__getset(0,__proto,'forward',function(){
		var worldMatElem=this.worldMatrix.elements;
		this._forward.elements[0]=-worldMatElem[8];
		this._forward.elements[1]=-worldMatElem[9];
		this._forward.elements[2]=-worldMatElem[10];
		return this._forward;
	});

	/**
	*获取向上方向。
	*@return 向上方向。
	*/
	__getset(0,__proto,'up',function(){
		var worldMatElem=this.worldMatrix.elements;
		this._up.elements[0]=worldMatElem[4];
		this._up.elements[1]=worldMatElem[5];
		this._up.elements[2]=worldMatElem[6];
		return this._up;
	});

	/**
	*获取向右方向。
	*@return 向右方向。
	*/
	__getset(0,__proto,'right',function(){
		var worldMatElem=this.worldMatrix.elements;
		this._right.elements[0]=worldMatElem[0];
		this._right.elements[1]=worldMatElem[1];
		this._right.elements[2]=worldMatElem[2];
		return this._right;
	});

	/**
	*设置父3D变换。
	*@param value 父3D变换。
	*/
	/**
	*获取父3D变换。
	*@return 父3D变换。
	*/
	__getset(0,__proto,'parent',function(){
		return this._parent;
		},function(value){
		if (this._parent!==value){
			if (this._parent){
				var parentChilds=this._parent._childs;
				var index=parentChilds.indexOf(this);
				parentChilds.splice(index,1);
			}
			if (value){
				value._childs.push(this);
				(value)&& (this._onWorldTransform());
			}
			this._parent=value;
		}
	});

	__static(Transform3D,
	['_tempVector30',function(){return this._tempVector30=new Vector3();},'_tempVector31',function(){return this._tempVector31=new Vector3();},'_tempVector32',function(){return this._tempVector32=new Vector3();},'_tempVector33',function(){return this._tempVector33=new Vector3();},'_tempQuaternion0',function(){return this._tempQuaternion0=new Quaternion();},'_tempMatrix0',function(){return this._tempMatrix0=new Matrix4x4();},'_angleToRandin',function(){return this._angleToRandin=180 / Math.PI;}
	]);
	return Transform3D;
})(EventDispatcher)


/**
*<code>TransformUV</code> 类用于实现UV变换。
*/
//class laya.d3.core.TransformUV extends laya.events.EventDispatcher
var TransformUV=(function(_super){
	function TransformUV(){
		/**@private */
		this._rotation=0;
		/**@private */
		//this._tiling=null;
		/**@private */
		this._matNeedUpdte=false;
		TransformUV.__super.call(this);
		this._matrix=new Matrix4x4();
		this._offset=new Vector2();
		this._tiling=new Vector2(1.0,1.0);
	}

	__class(TransformUV,'laya.d3.core.TransformUV',_super);
	var __proto=TransformUV.prototype;
	Laya.imps(__proto,{"laya.d3.core.IClone":true})
	/**
	*@private
	*/
	__proto._updateMatrix=function(){
		TransformUV._tempOffsetV3.elements[0]=this._offset.x;
		TransformUV._tempOffsetV3.elements[1]=this._offset.y;
		Quaternion.createFromYawPitchRoll(0,0,this._rotation,TransformUV._tempRotationQua);
		TransformUV._tempTitlingV3.elements[0]=this._tiling.x;
		TransformUV._tempTitlingV3.elements[1]=this._tiling.y;
		Matrix4x4.createAffineTransformation(TransformUV._tempOffsetV3,TransformUV._tempRotationQua,TransformUV._tempTitlingV3,this._matrix);
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		destObject._matrix=this._matrix.clone();
		destObject._offset=this._offset.clone();
		destObject._rotation=this._rotation;
		destObject._tiling=this._tiling.clone();
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var dest=/*__JS__ */new this.constructor();
		this.cloneTo(dest);
		return dest;
	}

	/**
	*获取变换矩阵。
	*@return 变换矩阵。
	*/
	__getset(0,__proto,'matrix',function(){
		if (this._matNeedUpdte){
			this._updateMatrix();
			this._matNeedUpdte=false;
		}
		return this._matrix;
	});

	/**
	*设置平铺次数。
	*@param value 平铺次数。
	*/
	/**
	*获取平铺次数。
	*@return 平铺次数。
	*/
	__getset(0,__proto,'tiling',function(){
		return this._tiling;
		},function(value){
		this._tiling=value;
		this._matNeedUpdte=true;
	});

	/**
	*设置偏移。
	*@param value 偏移。
	*/
	/**
	*获取偏移。
	*@return 偏移。
	*/
	__getset(0,__proto,'offset',function(){
		return this._offset;
		},function(value){
		this._offset=value;
		this._matNeedUpdte=true;
	});

	/**
	*设置旋转。
	*@param value 旋转。
	*/
	/**
	*获取旋转。
	*@return 旋转。
	*/
	__getset(0,__proto,'rotation',function(){
		return this._rotation;
		},function(value){
		this._rotation=value;
		this._matNeedUpdte=true;
	});

	__static(TransformUV,
	['_tempOffsetV3',function(){return this._tempOffsetV3=new Vector3(0,0,0);},'_tempRotationQua',function(){return this._tempRotationQua=new Quaternion();},'_tempTitlingV3',function(){return this._tempTitlingV3=new Vector3(1,1,1);}
	]);
	return TransformUV;
})(EventDispatcher)



/**
*@private
*/
//class laya.d3.core.render.SubMeshRenderElement extends laya.d3.core.render.RenderElement
var SubMeshRenderElement=(function(_super){
	function SubMeshRenderElement(){
		/**@private */
		this._batchIndexStart=0;
		/**@private */
		this._batchIndexEnd=0;
		/**@private */
		this._skinAnimationDatas=null;
		SubMeshRenderElement.__super.call(this);
	}

	__class(SubMeshRenderElement,'laya.d3.core.render.SubMeshRenderElement',_super);
	return SubMeshRenderElement;
})(RenderElement)


/**
*@private
*<code>ShaderCompile</code> 类用于创建Shader编译类型。
*/
//class laya.d3.shader.ShaderCompile3D extends laya.webgl.utils.ShaderCompile
var ShaderCompile3D=(function(_super){
	function ShaderCompile3D(name,vs,ps,attributeMap,uniformMap,includeFiles){
		/**@private */
		this._name=NaN;
		/**@private */
		this._attributeMap=null;
		/**@private */
		this._renderElementUniformMap=null;
		/**@private */
		this._materialUniformMap=null;
		/**@private */
		this._spriteUniformMap=null;
		/**@private */
		this._cameraUniformMap=null;
		/**@private */
		this._sceneUniformMap=null;
		this.sharders=null;
		/**@private */
		this._spriteDefineCounter=3;
		/**@private */
		this._spriteInt2name=[];
		/**@private */
		this._spriteName2Int={};
		/**@private */
		this._materialDefineCounter=1;
		/**@private */
		this._materialInt2name=[];
		/**@private */
		this._materialName2Int={};
		this._conchShader=null;
		this._name=name;
		this._renderElementUniformMap={};
		this._materialUniformMap={};
		this._spriteUniformMap={};
		this._cameraUniformMap={};
		this._sceneUniformMap={};
		this.sharders=[];
		this._spriteInt2name[RenderableSprite3D.SHADERDEFINE_SCALEOFFSETLIGHTINGMAPUV]="SCALEOFFSETLIGHTINGMAPUV";
		this._spriteInt2name[RenderableSprite3D.SAHDERDEFINE_LIGHTMAP]="LIGHTMAP";
		this._materialInt2name[BaseMaterial.SHADERDEFINE_ALPHATEST]="ALPHATEST";
		var defineMap={};
		ShaderCompile3D.__super.call(this,name,vs,ps,null,defineMap);
		this._attributeMap=attributeMap;
		var renderElementUnifCount=0,materialUnifCount=0,spriteUnifCount=0;
		var key;
		for (key in uniformMap){
			var uniformParam=uniformMap[key];
			switch (uniformParam[1]){
				case /*laya.d3.shader.Shader3D.PERIOD_RENDERELEMENT*/0:
					this._renderElementUniformMap[key]=uniformParam[0];
					break ;
				case /*laya.d3.shader.Shader3D.PERIOD_MATERIAL*/1:
					this._materialUniformMap[key]=uniformParam[0];
					break ;
				case /*laya.d3.shader.Shader3D.PERIOD_SPRITE*/2:
					this._spriteUniformMap[key]=uniformParam[0];
					break ;
				case /*laya.d3.shader.Shader3D.PERIOD_CAMERA*/3:
					this._cameraUniformMap[key]=uniformParam[0];
					break ;
				case /*laya.d3.shader.Shader3D.PERIOD_SCENE*/4:
					this._sceneUniformMap[key]=uniformParam[0];
					break ;
				default :
					throw new Error("ShaderCompile3D: period is unkonw.");
				}
		}
	}

	__class(ShaderCompile3D,'laya.d3.shader.ShaderCompile3D',_super);
	var __proto=ShaderCompile3D.prototype;
	/**
	*@private
	*/
	__proto._definesToNameDic=function(value,int2Name){
		var o={};
		var d=1;
		for (var i=0;i < 32;i++){
			d=1 << i;
			if (d > value)break ;
			if (value & d){
				var name=int2Name[d];
				name && (o[name]="");
			}
		}
		return o;
	}

	/**
	*根据宏动态生成shader文件，支持#include?COLOR_FILTER "parts/ColorFilter_ps_logic.glsl";条件嵌入文件
	*@param name
	*@param vs
	*@param ps
	*@param define 宏定义，格式:{name:value...}
	*@return
	*/
	__proto.withCompile=function(publicDefine,spriteDefine,materialDefine){
		var shader;
		var spriteDefShaders,materialDefShaders;
		spriteDefShaders=this.sharders[publicDefine];
		if (spriteDefShaders){
			materialDefShaders=spriteDefShaders[spriteDefine];
			if (materialDefShaders){
				shader=materialDefShaders[materialDefine];
				if (shader)
					return shader;
				}else {
				materialDefShaders=spriteDefShaders[spriteDefine]=[];
			}
			}else {
			spriteDefShaders=this.sharders[publicDefine]=[];
			materialDefShaders=spriteDefShaders[spriteDefine]=[];
		};
		var publicDefGroup=this._definesToNameDic(publicDefine,ShaderCompile3D._globalInt2name);
		var spriteDefGroup=this._definesToNameDic(spriteDefine,this._spriteInt2name);
		var materialDefGroup=this._definesToNameDic(materialDefine,this._materialInt2name);
		var key;
		if (laya.d3.shader.ShaderCompile3D.debugMode){
			var publicDefGroupStr="";
			for (key in publicDefGroup)
			publicDefGroupStr+=key+" ";
			var spriteDefGroupStr="";
			for (key in spriteDefGroup)
			spriteDefGroupStr+=key+" ";
			var materialDefGroupStr="";
			for (key in materialDefGroup)
			materialDefGroupStr+=key+" ";
			console.log("ShaderCompile3DDebugMode---(Name:"+Shader3D.nameKey.getName(this._name)+" PublicDefine:"+publicDefine+" SpriteDefine:"+spriteDefine+" MaterialDefine:"+materialDefine+" PublicDefineGroup:"+publicDefGroupStr+" SpriteDefineGroup:"+spriteDefGroupStr+"MaterialDefineGroup: "+materialDefGroupStr+")---ShaderCompile3DDebugMode");
		};
		var defMap={};
		var defineStr="";
		if (publicDefGroup){
			for (key in publicDefGroup){
				defineStr+="#define "+key+"\n";
				defMap[key]=true;
			}
		}
		if (spriteDefGroup){
			for (key in spriteDefGroup){
				defineStr+="#define "+key+"\n";
				defMap[key]=true;
			}
		}
		if (materialDefGroup){
			for (key in materialDefGroup){
				defineStr+="#define "+key+"\n";
				defMap[key]=true;
			}
		};
		var vs=this._VS.toscript(defMap,[]);
		var ps=this._PS.toscript(defMap,[]);
		shader=Shader3D.create(defineStr+vs.join('\n'),defineStr+ps.join('\n'),this._attributeMap,this._sceneUniformMap,this._cameraUniformMap,this._spriteUniformMap,this._materialUniformMap,this._renderElementUniformMap);
		materialDefShaders[materialDefine]=shader;
		return shader;
	}

	/**
	*通过宏定义预编译shader。
	*@param spriteIntToNameDic 精灵宏定义数组。
	*@param publicDefine 公共宏定义值。
	*@param spriteDefine 精灵宏定义值。
	*@param materialDefine 材质宏定义值。
	*/
	__proto.precompileShaderWithShaderDefine=function(publicDefine,spriteDefine,materialDefine){
		this.withCompile(publicDefine,spriteDefine,materialDefine);
	}

	/**
	*注册材质宏定义。
	*@param name 宏定义名称。
	*@return
	*/
	__proto.addMaterialDefines=function(shaderdefines){
		var defines=shaderdefines.defines;
		for (var k in defines){
			var name=defines[k];
			var i=parseInt(k);
			this._materialInt2name[i]=name;
			this._materialName2Int[name]=i;
		}
	}

	/**
	*注册精灵宏定义。
	*@param name 宏定义名称。
	*@return
	*/
	__proto.addSpriteDefines=function(shaderdefines){
		var defines=shaderdefines.defines;
		for (var k in defines){
			var name=defines[k];
			var i=parseInt(k);
			this._spriteInt2name[i]=name;
			this._spriteName2Int[name]=i;
		}
	}

	/**
	*通过名称获取宏定义值。
	*@param name 名称。
	*@return 宏定义值。
	*/
	__proto.getMaterialDefineByName=function(name){
		return this._materialName2Int[name];
	}

	/**
	*注册材质宏定义。
	*@param name 宏定义名称。
	*@return
	*/
	__proto.registerMaterialDefine=function(name){
		var value=Math.pow(2,this._materialDefineCounter++);
		this._materialInt2name[value]=name;
		this._materialName2Int[name]=value;
		return value;
	}

	/**
	*注册精灵宏定义。
	*@param name 宏定义名称。
	*@return
	*/
	__proto.registerSpriteDefine=function(name){
		var value=Math.pow(2,this._spriteDefineCounter++);
		this._spriteInt2name[value]=name;
		this._spriteName2Int[name]=value;
		return value;
	}

	ShaderCompile3D._globalRegDefine=function(name,value){
		ShaderCompile3D._globalInt2name[value]=name;
	}

	ShaderCompile3D.add=function(nameID,vs,ps,attributeMap,uniformMap){
		return laya.d3.shader.ShaderCompile3D._preCompileShader[nameID]=new ShaderCompile3D(nameID,vs,ps,attributeMap,uniformMap,ShaderCompile.includes);
	}

	ShaderCompile3D.get=function(name){
		return laya.d3.shader.ShaderCompile3D._preCompileShader[Shader3D.nameKey.getID(name)];
	}

	ShaderCompile3D._preCompileShader={};
	ShaderCompile3D._globalInt2name=[];
	ShaderCompile3D.debugMode=false;
	ShaderCompile3D.SHADERDEFINE_HIGHPRECISION=0x1;
	ShaderCompile3D.SHADERDEFINE_FOG=0x4;
	ShaderCompile3D.SHADERDEFINE_DIRECTIONLIGHT=0x8;
	ShaderCompile3D.SHADERDEFINE_POINTLIGHT=0x10;
	ShaderCompile3D.SHADERDEFINE_SPOTLIGHT=0x20;
	ShaderCompile3D.SHADERDEFINE_UV0=0x40;
	ShaderCompile3D.SHADERDEFINE_COLOR=0x80;
	ShaderCompile3D.SHADERDEFINE_UV1=0x100;
	ShaderCompile3D.SAHDERDEFINE_DEPTHFOG=0x20000;
	return ShaderCompile3D;
})(ShaderCompile)


/**
*@private
*<code>ComponentNode</code> 类用于实现组件精灵,该类为抽象类。
*/
//class laya.d3.core.ComponentNode extends laya.display.Node
var ComponentNode=(function(_super){
	function ComponentNode(){
		/**@private */
		this._componentsMap=null;
		/**@private */
		this._typeComponentsIndices=null;
		/**@private */
		this._components=null;
		/**@private */
		this._scripts=null;
		ComponentNode.__super.call(this);
		this._componentsMap=[];
		this._typeComponentsIndices=[];
		this._components=[];
		this._scripts=[];
	}

	__class(ComponentNode,'laya.d3.core.ComponentNode',_super);
	var __proto=ComponentNode.prototype;
	/**
	*添加指定类型组件。
	*@param type 组件类型。
	*@return 组件。
	*/
	__proto.addComponent=function(type){
		var typeComponentIndex;
		var index=this._componentsMap.indexOf(type);
		if (index===-1){
			typeComponentIndex=[];
			this._componentsMap.push(type);
			this._typeComponentsIndices.push(typeComponentIndex);
			}else {
			typeComponentIndex=this._typeComponentsIndices[index];
			if (this._components[typeComponentIndex[0]].isSingleton)
				throw new Error("无法单实例创建"+type+"组件"+"，"+type+"组件已存在！");
		};
		var component=ClassUtils.getInstance(type);
		typeComponentIndex.push(this._components.length);
		this._components.push(component);
		if ((component instanceof laya.d3.component.Script ))
			this._scripts.push(component);
		component._initialize(this);
		return component;
	}

	/**
	*@private
	*/
	__proto._removeComponent=function(mapIndex,index){
		var componentIndices=this._typeComponentsIndices[mapIndex];
		var componentIndex=componentIndices[index];
		var component=this._components[componentIndex];
		this._components.splice(componentIndex,1);
		if ((component instanceof laya.d3.component.Script ))
			this._scripts.splice(this._scripts.indexOf(component),1);
		componentIndices.splice(index,1);
		(componentIndices.length===0)&& (this._typeComponentsIndices.splice(mapIndex,1),this._componentsMap.splice(mapIndex,1));
		for (var i=0,n=this._componentsMap.length;i < n;i++){
			componentIndices=this._typeComponentsIndices[i];
			for (var j=componentIndices.length-1;j >=0;j--){
				var oldComponentIndex=componentIndices[j];
				if (oldComponentIndex > componentIndex)
					componentIndices[j]=--oldComponentIndex;
				else
				break ;
			}
		}
		component._destroy();
	}

	/**
	*通过指定类型和类型索引获得组件。
	*@param type 组件类型。
	*@param typeIndex 类型索引。
	*@return 组件。
	*/
	__proto.getComponentByType=function(type,typeIndex){
		(typeIndex===void 0)&& (typeIndex=0);
		var mapIndex=this._componentsMap.indexOf(type);
		if (mapIndex===-1)
			return null;
		return this._components[this._typeComponentsIndices[mapIndex][typeIndex]];
	}

	/**
	*通过指定类型获得所有组件。
	*@param type 组件类型。
	*@param components 组件输出队列。
	*/
	__proto.getComponentsByType=function(type,components){
		var index=this._componentsMap.indexOf(type);
		if (index===-1){
			components.length=0;
			return;
		};
		var typeComponents=this._typeComponentsIndices[index];
		var count=typeComponents.length;
		components.length=count;
		for (var i=0;i < count;i++)
		components[i]=this._components[typeComponents[i]];
	}

	/**
	*通过指定索引获得组件。
	*@param index 索引。
	*@return 组件。
	*/
	__proto.getComponentByIndex=function(index){
		return this._components[index];
	}

	/**
	*通过指定类型和类型索引移除组件。
	*@param type 组件类型。
	*@param typeIndex 类型索引。
	*/
	__proto.removeComponentByType=function(type,typeIndex){
		(typeIndex===void 0)&& (typeIndex=0);
		var mapIndex=this._componentsMap.indexOf(type);
		if (mapIndex===-1)
			return;
		this._removeComponent(mapIndex,typeIndex);
	}

	/**
	*通过指定类型移除所有组件。
	*@param type 组件类型。
	*/
	__proto.removeComponentsByType=function(type){
		var mapIndex=this._componentsMap.indexOf(type);
		if (mapIndex===-1)
			return;
		var componentIndices=this._typeComponentsIndices[mapIndex];
		for (var i=0,n=componentIndices.length;i < n;componentIndices.length < n ? n--:i++)
		this._removeComponent(mapIndex,i);
	}

	/**
	*移除全部组件。
	*/
	__proto.removeAllComponent=function(){
		for (var i=0,n=this._componentsMap.length;i < n;this._componentsMap.length < n ? n--:i++)
		this.removeComponentsByType(this._componentsMap[i]);
	}

	/**
	*@private
	*/
	__proto._updateComponents=function(state){
		for (var i=0,n=this._components.length;i < n;i++){
			var component=this._components[i];
			(!component.started)&& (component._start(state),component.started=true);
			(component.enable)&& (component._update(state));
		}
	}

	/**
	*@private
	*/
	__proto._lateUpdateComponents=function(state){
		for (var i=0;i < this._components.length;i++){
			var component=this._components[i];
			(!component.started)&& (component._start(state),component.started=true);
			(component.enable)&& (component._lateUpdate(state));
		}
	}

	/**
	*@private
	*/
	__proto._preRenderUpdateComponents=function(state){
		for (var i=0;i < this._components.length;i++){
			var component=this._components[i];
			(!component.started)&& (component._start(state),component.started=true);
			(component.enable)&& (component._preRenderUpdate(state));
		}
	}

	/**
	*@private
	*/
	__proto._postRenderUpdateComponents=function(state){
		for (var i=0;i < this._components.length;i++){
			var component=this._components[i];
			(!component.started)&& (component._start(state),component.started=true);
			(component.enable)&& (component._postRenderUpdate(state));
		}
	}

	return ComponentNode;
})(Node)


/**
*<code>BaseMaterial</code> 类用于创建材质,抽象类,不允许实例。
*/
//class laya.d3.core.material.BaseMaterial extends laya.resource.Resource
var BaseMaterial=(function(_super){
	function BaseMaterial(){
		/**@private */
		//this._shader=null;
		/**@private */
		//this._shaderCompile=null;
		/**@private */
		//this._shaderDefineValue=0;
		/**@private */
		//this._disablePublicShaderDefine=0;
		/**@private */
		//this._alphaTest=false;
		/**@private */
		//this._shaderValues=null;
		/**@private */
		//this._values=null;
		/**渲染剔除状态。*/
		//this.cull=0;
		/**透明混合。*/
		//this.blend=0;
		/**源混合参数,在blend为BLEND_ENABLE_ALL时生效。*/
		//this.srcBlend=0;
		/**目标混合参数,在blend为BLEND_ENABLE_ALL时生效。*/
		//this.dstBlend=0;
		/**RGB源混合参数,在blend为BLEND_ENABLE_SEPERATE时生效。*/
		//this.srcBlendRGB=0;
		/**RGB目标混合参数,在blend为BLEND_ENABLE_SEPERATE时生效。*/
		//this.dstBlendRGB=0;
		/**Alpha源混合参数,在blend为BLEND_ENABLE_SEPERATE时生效。*/
		//this.srcBlendAlpha=0;
		/**Alpha目标混合参数,在blend为BLEND_ENABLE_SEPERATE时生效。*/
		//this.dstBlendAlpha=0;
		/**混合常量颜色。*/
		//this.blendConstColor=null;
		/**混合方程。*/
		//this.blendEquation=0;
		/**RGB混合方程。*/
		//this.blendEquationRGB=0;
		/**Alpha混合方程。*/
		//this.blendEquationAlpha=0;
		/**深度测试函数。*/
		//this.depthTest=0;
		/**是否深度写入。*/
		//this.depthWrite=false;
		/**所属渲染队列. */
		//this.renderQueue=0;
		/**@private */
		//this._conchMaterial=null;
		BaseMaterial.__super.call(this);
		this._shaderDefineValue=0;
		this._disablePublicShaderDefine=0;
		this._shaderValues=new ValusArray();
		this._values=[];
		this.renderQueue=/*laya.d3.core.render.RenderQueue.OPAQUE*/1;
		this._alphaTest=false;
		this.cull=2;
		this.blend=0;
		this.srcBlend=1;
		this.dstBlend=0;
		this.srcBlendRGB=1;
		this.dstBlendRGB=0;
		this.srcBlendAlpha=1;
		this.dstBlendAlpha=0;
		this.blendConstColor=new Vector4(1,1,1,1);
		this.blendEquation=0;
		this.blendEquationRGB=0;
		this.blendEquationAlpha=0;
		this.depthTest=0x0201;
		this.depthWrite=true;
	}

	__class(BaseMaterial,'laya.d3.core.material.BaseMaterial',_super);
	var __proto=BaseMaterial.prototype;
	Laya.imps(__proto,{"laya.d3.core.IClone":true})
	/**
	*增加Shader宏定义。
	*@param value 宏定义。
	*/
	__proto._addShaderDefine=function(value){
		this._shaderDefineValue |=value;
	}

	/**
	*移除Shader宏定义。
	*@param value 宏定义。
	*/
	__proto._removeShaderDefine=function(value){
		this._shaderDefineValue &=~value;
	}

	/**
	*增加禁用宏定义。
	*@param value 宏定义。
	*/
	__proto._addDisablePublicShaderDefine=function(value){
		this._disablePublicShaderDefine |=value;
	}

	/**
	*移除禁用宏定义。
	*@param value 宏定义。
	*/
	__proto._removeDisablePublicShaderDefine=function(value){
		this._disablePublicShaderDefine &=~value;
	}

	/**
	*设置Buffer。
	*@param shaderIndex shader索引。
	*@param buffer buffer数据。
	*/
	__proto._setBuffer=function(shaderIndex,buffer){
		var shaderValue=this._shaderValues;
		shaderValue.setValue(shaderIndex,buffer);
		this._values[shaderIndex]=buffer;
	}

	/**
	*获取Buffer。
	*@param shaderIndex shader索引。
	*@return
	*/
	__proto._getBuffer=function(shaderIndex){
		return this._values[shaderIndex];
	}

	/**
	*设置矩阵。
	*@param shaderIndex shader索引。
	*@param matrix4x4 矩阵。
	*/
	__proto._setMatrix4x4=function(shaderIndex,matrix4x4){
		this._shaderValues.setValue(shaderIndex,matrix4x4 ? matrix4x4.elements :null);
		this._values[shaderIndex]=matrix4x4;
	}

	/**
	*获取矩阵。
	*@param shaderIndex shader索引。
	*@return 矩阵。
	*/
	__proto._getMatrix4x4=function(shaderIndex){
		return this._values[shaderIndex];
	}

	/**
	*设置整型。
	*@param shaderIndex shader索引。
	*@param i 整形。
	*/
	__proto._setInt=function(shaderIndex,i){
		var shaderValue=this._shaderValues;
		shaderValue.setValue(shaderIndex,i);
		this._values[shaderIndex]=i;
	}

	/**
	*获取整形。
	*@param shaderIndex shader索引。
	*@return 整形。
	*/
	__proto._getInt=function(shaderIndex){
		return this._values[shaderIndex];
	}

	/**
	*设置浮点。
	*@param shaderIndex shader索引。
	*@param i 浮点。
	*/
	__proto._setNumber=function(shaderIndex,number){
		var shaderValue=this._shaderValues;
		shaderValue.setValue(shaderIndex,number);
		this._values[shaderIndex]=number;
	}

	/**
	*获取浮点。
	*@param shaderIndex shader索引。
	*@return 浮点。
	*/
	__proto._getNumber=function(shaderIndex){
		return this._values[shaderIndex];
	}

	/**
	*设置布尔。
	*@param shaderIndex shader索引。
	*@param b 布尔。
	*/
	__proto._setBool=function(shaderIndex,b){
		var shaderValue=this._shaderValues;
		shaderValue.setValue(shaderIndex,b);
		this._values[shaderIndex]=b;
	}

	/**
	*获取布尔。
	*@param shaderIndex shader索引。
	*@return 布尔。
	*/
	__proto._getBool=function(shaderIndex){
		return this._values[shaderIndex];
	}

	/**
	*设置二维向量。
	*@param shaderIndex shader索引。
	*@param vector2 二维向量。
	*/
	__proto._setVector2=function(shaderIndex,vector2){
		var shaderValue=this._shaderValues;
		shaderValue.setValue(shaderIndex,vector2 ? vector2.elements :null);
		this._values[shaderIndex]=vector2;
	}

	/**
	*获取二维向量。
	*@param shaderIndex shader索引。
	*@return 二维向量。
	*/
	__proto._getVector2=function(shaderIndex){
		return this._values[shaderIndex];
	}

	/**
	*设置颜色。
	*@param shaderIndex shader索引。
	*@param color 颜色向量。
	*/
	__proto._setColor=function(shaderIndex,color){
		var shaderValue=this._shaderValues;
		shaderValue.setValue(shaderIndex,color ? color.elements :null);
		this._values[shaderIndex]=color;
	}

	/**
	*获取颜色。
	*@param shaderIndex shader索引。
	*@return 颜色向量。
	*/
	__proto._getColor=function(shaderIndex){
		return this._values[shaderIndex];
	}

	/**
	*设置纹理。
	*@param shaderIndex shader索引。
	*@param texture 纹理。
	*/
	__proto._setTexture=function(shaderIndex,texture){
		var lastValue=this._values[shaderIndex];
		this._values[shaderIndex]=texture;
		this._shaderValues.setValue(shaderIndex,texture);
		if (this.referenceCount > 0){
			(lastValue)&& (lastValue._removeReference());
			(texture)&& (texture._addReference());
		}
	}

	/**
	*获取纹理。
	*@param shaderIndex shader索引。
	*@return 纹理。
	*/
	__proto._getTexture=function(shaderIndex){
		return this._values[shaderIndex];
	}

	/**
	*上传材质。
	*@param state 相关渲染状态。
	*@param bufferUsageShader Buffer相关绑定。
	*@param shader 着色器。
	*@return 是否成功。
	*/
	__proto._upload=function(){
		this._shader.uploadMaterialUniforms(this._shaderValues.data);
	}

	/**
	*@private
	*/
	__proto._getShader=function(sceneDefineValue,vertexDefineValue,spriteDefineValue){
		var publicDefineValue=(sceneDefineValue | vertexDefineValue)& (~this._disablePublicShaderDefine);
		this._shader=this._shaderCompile.withCompile(publicDefineValue,spriteDefineValue,this._shaderDefineValue);
		return this._shader;
	}

	/**
	*设置渲染相关状态。
	*/
	__proto._setRenderStateBlendDepth=function(){
		var gl=WebGL.mainContext;
		WebGLContext.setDepthMask(gl,this.depthWrite);
		if (this.depthTest===0)
			WebGLContext.setDepthTest(gl,false);
		else {
			WebGLContext.setDepthTest(gl,true);
			WebGLContext.setDepthFunc(gl,this.depthTest);
		}
		switch (this.blend){
			case 0:
				WebGLContext.setBlend(gl,false);
				break ;
			case 1:
				WebGLContext.setBlend(gl,true);
				WebGLContext.setBlendFunc(gl,this.srcBlend,this.dstBlend);
				break ;
			case 2:
				WebGLContext.setBlend(gl,true);
				break ;
			}
	}

	/**
	*设置渲染相关状态。
	*/
	__proto._setRenderStateFrontFace=function(isTarget,transform){
		var gl=WebGL.mainContext;
		var forntFace=0;
		switch (this.cull){
			case 0:
				WebGLContext.setCullFace(gl,false);
				break ;
			case 1:
				WebGLContext.setCullFace(gl,true);
				if (isTarget){
					if (transform && transform._isFrontFaceInvert)
						forntFace=/*laya.webgl.WebGLContext.CCW*/0x0901;
					else
					forntFace=/*laya.webgl.WebGLContext.CW*/0x0900;
					}else {
					if (transform && transform._isFrontFaceInvert)
						forntFace=/*laya.webgl.WebGLContext.CW*/0x0900;
					else
					forntFace=/*laya.webgl.WebGLContext.CCW*/0x0901;
				}
				WebGLContext.setFrontFace(gl,forntFace);
				break ;
			case 2:
				WebGLContext.setCullFace(gl,true);
				if (isTarget){
					if (transform && transform._isFrontFaceInvert)
						forntFace=/*laya.webgl.WebGLContext.CW*/0x0900;
					else
					forntFace=/*laya.webgl.WebGLContext.CCW*/0x0901;
					}else {
					if (transform && transform._isFrontFaceInvert)
						forntFace=/*laya.webgl.WebGLContext.CCW*/0x0901;
					else
					forntFace=/*laya.webgl.WebGLContext.CW*/0x0900;
				}
				WebGLContext.setFrontFace(gl,forntFace);
				break ;
			}
	}

	/**
	*@inheritDoc
	*/
	__proto.onAsynLoaded=function(url,data,params){
		var jsonData=data[0];
		var textureMap=data[1];
		switch (jsonData.version){
			case "LAYAMATERIAL:01":;
				var i=0,n=0;
				var props=jsonData.props;
				for (var key in props){
				switch (key){
					case "vectors":;
						var vectors=props[key];
						for (i=0,n=vectors.length;i < n;i++){
							var vector=vectors[i];
							var vectorValue=vector.value;
						switch (vectorValue.length){
							case 2:
								this[vector.name]=new Vector2(vectorValue[0],vectorValue[1]);
								break ;
							case 3:
								this[vector.name]=new Vector3(vectorValue[0],vectorValue[1],vectorValue[2]);
								break ;
							case 4:
								this[vector.name]=new Vector4(vectorValue[0],vectorValue[1],vectorValue[2],vectorValue[3]);
								break ;
							default :
								throw new Error("BaseMaterial:unkonwn color length.");
							}
					}
					break ;
					case "textures":;
					var textures=props[key];
					for (i=0,n=textures.length;i < n;i++){
						var texture=textures[i];
						var path=texture.path;
						(path)&& (this[texture.name]=Loader.getRes(textureMap[path]));
					}
					break ;
					case "defines":;
					var defineNames=props[key];
					for (i=0,n=defineNames.length;i < n;i++){
						var define=this._shaderCompile.getMaterialDefineByName(defineNames[i]);
						this._addShaderDefine(define);
					}
					break ;
					default :
					this[key]=props[key];
				}
			}
			break ;
			default :
			throw new Error("BaseMaterial:unkonwn version.");
		}
		this._endLoaded();
	}

	/**
	*@inheritDoc
	*/
	__proto._addReference=function(){
		_super.prototype._addReference.call(this);
		var valueCount=this._values.length;
		for (var i=0,n=valueCount;i < n;i++){
			var value=this._values[i];
			if (value && (value instanceof laya.d3.resource.BaseTexture ))
				(value)._addReference();
		}
	}

	/**
	*@inheritDoc
	*/
	__proto._removeReference=function(){
		_super.prototype._removeReference.call(this);
		var valueCount=this._values.length;
		for (var i=0,n=valueCount;i < n;i++){
			var value=this._values[i];
			if (value && (value instanceof laya.d3.resource.BaseTexture ))
				(value)._removeReference();
		}
	}

	/**
	*@inheritDoc
	*/
	__proto.disposeResource=function(){
		this.blendConstColor=null;
		this._shader=null;
		this._shaderValues=null;
		var valueCount=this._values.length;
		for (var i=0,n=valueCount;i < n;i++){
			var value=this._values[i];
			if (value && (value instanceof laya.d3.resource.BaseTexture ))
				(value)._removeReference();
		}
		this._values=null;
	}

	/**
	*设置使用Shader名字。
	*@param name 名称。
	*/
	__proto.setShaderName=function(name){
		var nameID=Shader3D.nameKey.getID(name);
		if (nameID===-1)
			throw new Error("BaseMaterial: unknown shader name.");
		this._shaderCompile=ShaderCompile3D._preCompileShader[nameID];
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		var destBaseMaterial=destObject;
		destBaseMaterial.name=this.name;
		destBaseMaterial.cull=this.cull;
		destBaseMaterial.blend=this.blend;
		destBaseMaterial.srcBlend=this.srcBlend;
		destBaseMaterial.dstBlend=this.dstBlend;
		destBaseMaterial.srcBlendRGB=this.srcBlendRGB;
		destBaseMaterial.dstBlendRGB=this.dstBlendRGB;
		destBaseMaterial.srcBlendAlpha=this.srcBlendAlpha;
		destBaseMaterial.dstBlendAlpha=this.dstBlendAlpha;
		this.blendConstColor.cloneTo(destBaseMaterial.blendConstColor);
		destBaseMaterial.blendEquation=this.blendEquation;
		destBaseMaterial.blendEquationRGB=this.blendEquationRGB;
		destBaseMaterial.blendEquationAlpha=this.blendEquationAlpha;
		destBaseMaterial.depthTest=this.depthTest;
		destBaseMaterial.depthWrite=this.depthWrite;
		destBaseMaterial.renderQueue=this.renderQueue;
		destBaseMaterial._shader=this._shader;
		destBaseMaterial._disablePublicShaderDefine=this._disablePublicShaderDefine;
		destBaseMaterial._shaderDefineValue=this._shaderDefineValue;
		var i=0,n=0;
		var destShaderValues=destBaseMaterial._shaderValues;
		destBaseMaterial._shaderValues.data.length=this._shaderValues.data.length;
		var valueCount=this._values.length;
		var destValues=destBaseMaterial._values;
		destValues.length=valueCount;
		for (i=0,n=valueCount;i < n;i++){
			var value=this._values[i];
			if (value){
				if ((typeof value=='number')){
					destValues[i]=value;
					destShaderValues.data[i]=value;
					}else if (((typeof value=='number')&& Math.floor(value)==value)){
					destValues[i]=value;
					destShaderValues.data[i]=value;
					}else if ((typeof value=='boolean')){
					destValues[i]=value;
					destShaderValues.data[i]=value;
					}else if ((value instanceof laya.d3.math.Vector2 )){
					var v2=(destValues[i])|| (destValues[i]=new Vector2());
					(value).cloneTo(v2);
					destShaderValues.data[i]=v2.elements;
					}else if ((value instanceof laya.d3.math.Vector3 )){
					var v3=(destValues[i])|| (destValues[i]=new Vector3());
					(value).cloneTo(v3);
					destShaderValues.data[i]=v3.elements;
					}else if ((value instanceof laya.d3.math.Vector4 )){
					var v4=(destValues[i])|| (destValues[i]=new Vector4());
					(value).cloneTo(v4);
					destShaderValues.data[i]=v4.elements;
					}else if ((value instanceof laya.d3.math.Matrix4x4 )){
					var mat=(destValues[i])|| (destValues[i]=new Matrix4x4());
					(value).cloneTo(mat);
					destShaderValues.data[i]=mat.elements;
					}else if ((value instanceof laya.d3.resource.BaseTexture )){
					destValues[i]=value;
					destShaderValues.data[i]=value;
				}
			}
		}
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var destBaseMaterial=/*__JS__ */new this.constructor();
		this.cloneTo(destBaseMaterial);
		return destBaseMaterial;
	}

	/**
	*设置透明测试模式裁剪值。
	*@param value 透明测试模式裁剪值。
	*/
	/**
	*获取透明测试模式裁剪值。
	*@return 透明测试模式裁剪值。
	*/
	__getset(0,__proto,'alphaTestValue',function(){
		return this._getNumber(0);
		},function(value){
		this._setNumber(0,value);
	});

	/**
	*设置是否透明裁剪。
	*@param value 是否透明裁剪。
	*/
	/**
	*获取是否透明裁剪。
	*@return 是否透明裁剪。
	*/
	__getset(0,__proto,'alphaTest',function(){
		return this._alphaTest;
		},function(value){
		this._alphaTest=value;
		if (value)
			this._addShaderDefine(laya.d3.core.material.BaseMaterial.SHADERDEFINE_ALPHATEST);
		else
		this._removeShaderDefine(laya.d3.core.material.BaseMaterial.SHADERDEFINE_ALPHATEST);
	});

	BaseMaterial.__init__=function(){
		BaseMaterial.SHADERDEFINE_ALPHATEST=BaseMaterial.shaderDefines.registerDefine("ALPHATEST");
	}

	BaseMaterial.CULL_NONE=0;
	BaseMaterial.CULL_FRONT=1;
	BaseMaterial.CULL_BACK=2;
	BaseMaterial.BLEND_DISABLE=0;
	BaseMaterial.BLEND_ENABLE_ALL=1;
	BaseMaterial.BLEND_ENABLE_SEPERATE=2;
	BaseMaterial.BLENDPARAM_ZERO=0;
	BaseMaterial.BLENDPARAM_ONE=1;
	BaseMaterial.BLENDPARAM_SRC_COLOR=0x0300;
	BaseMaterial.BLENDPARAM_ONE_MINUS_SRC_COLOR=0x0301;
	BaseMaterial.BLENDPARAM_DST_COLOR=0x0306;
	BaseMaterial.BLENDPARAM_ONE_MINUS_DST_COLOR=0x0307;
	BaseMaterial.BLENDPARAM_SRC_ALPHA=0x0302;
	BaseMaterial.BLENDPARAM_ONE_MINUS_SRC_ALPHA=0x0303;
	BaseMaterial.BLENDPARAM_DST_ALPHA=0x0304;
	BaseMaterial.BLENDPARAM_ONE_MINUS_DST_ALPHA=0x0305;
	BaseMaterial.BLENDPARAM_SRC_ALPHA_SATURATE=0x0308;
	BaseMaterial.BLENDEQUATION_ADD=0;
	BaseMaterial.BLENDEQUATION_SUBTRACT=1;
	BaseMaterial.BLENDEQUATION_REVERSE_SUBTRACT=2;
	BaseMaterial.DEPTHTEST_OFF=0;
	BaseMaterial.DEPTHTEST_NEVER=0x0200;
	BaseMaterial.DEPTHTEST_LESS=0x0201;
	BaseMaterial.DEPTHTEST_EQUAL=0x0202;
	BaseMaterial.DEPTHTEST_LEQUAL=0x0203;
	BaseMaterial.DEPTHTEST_GREATER=0x0204;
	BaseMaterial.DEPTHTEST_NOTEQUAL=0x0205;
	BaseMaterial.DEPTHTEST_GEQUAL=0x0206;
	BaseMaterial.DEPTHTEST_ALWAYS=0x0207;
	BaseMaterial.SHADERDEFINE_ALPHATEST=0x1;
	BaseMaterial.ALPHATESTVALUE=0;
	__static(BaseMaterial,
	['shaderDefines',function(){return this.shaderDefines=new ShaderDefines$1();}
	]);
	return BaseMaterial;
})(Resource)


/**
*<code>BaseTexture</code> 纹理的父类，抽象类，不允许实例。
*/
//class laya.d3.resource.BaseTexture extends laya.resource.Resource
var BaseTexture=(function(_super){
	function BaseTexture(){
		/**@private */
		this._type=0;
		/**@private */
		this._width=0;
		/**@private */
		this._height=0;
		/**@private */
		this._size=null;
		/**@private */
		this._repeat=false;
		/**@private */
		this._mipmap=false;
		/**@private */
		this._minFifter=0;
		/**@private */
		this._magFifter=0;
		/**@private */
		this._format=0;
		/**@private */
		this._source=null;
		BaseTexture.__super.call(this);
		this._conchTexture
		this._repeat=true;
		this.mipmap=true;
		this.minFifter=-1;
		this.magFifter=-1;
	}

	__class(BaseTexture,'laya.d3.resource.BaseTexture',_super);
	var __proto=BaseTexture.prototype;
	/**
	*获取宽度。
	*/
	__getset(0,__proto,'width',function(){
		return this._width;
	});

	/**
	*纹理格式
	*/
	__getset(0,__proto,'format',function(){
		return this._format;
	});

	/**
	*是否使用重复模式纹理寻址
	*/
	/**
	*是否使用重复模式纹理寻址
	*/
	__getset(0,__proto,'repeat',function(){
		return this._repeat;
		},function(value){
		if (this._repeat!==value){
			this._repeat=value;
			if (this._source){
				var gl=WebGL.mainContext;
				WebGLContext.bindTexture(gl,this._type,this._source);
				var isPot=Arith.isPOT(this._width,this._height);
				if (isPot && this._repeat){
					gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_S*/0x2802,/*laya.webgl.WebGLContext.REPEAT*/0x2901);
					gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_T*/0x2803,/*laya.webgl.WebGLContext.REPEAT*/0x2901);
					}else {
					gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_S*/0x2802,/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F);
					gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_T*/0x2803,/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F);
				}
			}
		}
	});

	/**
	*获取高度。
	*/
	__getset(0,__proto,'height',function(){
		return this._height;
	});

	/**
	*放大过滤器
	*/
	/**
	*放大过滤器
	*/
	__getset(0,__proto,'magFifter',function(){
		return this._magFifter;
		},function(value){
		this._magFifter=value;
		if (value !=this._magFifter){
			this._conchTexture && this._conchTexture.setMaxFifter(value);
		}
	});

	/**
	*获取尺寸。
	*/
	__getset(0,__proto,'size',function(){
		return this._size;
	});

	/**
	*是否使用mipLevel
	*/
	/**
	*是否使用mipLevel
	*/
	__getset(0,__proto,'mipmap',function(){
		return this._mipmap;
		},function(value){
		this._mipmap=value;
		if (this._mipmap !=value){
			this._conchTexture && this._conchTexture.setMipMap(value);
		}
	});

	/**
	*缩小过滤器
	*/
	/**
	*缩小过滤器
	*/
	__getset(0,__proto,'minFifter',function(){
		return this._minFifter;
		},function(value){
		this._minFifter=value;
		if (this._minFifter !=value){
			this._conchTexture && this._conchTexture.setMinFifter(value);
		}
	});

	/**
	*获取纹理资源。
	*/
	__getset(0,__proto,'source',function(){
		this.activeResource();
		return this._source;
	});

	/**
	*获取纹理资源。
	*/
	__getset(0,__proto,'defaulteTexture',function(){
		return SolidColorTexture2D.grayTexture;
	});

	return BaseTexture;
})(Resource)


/**
*<code>BaseMesh</code> 类用于创建网格,抽象类,不允许实例。
*/
//class laya.d3.resource.models.BaseMesh extends laya.resource.Resource
var BaseMesh=(function(_super){
	function BaseMesh(){
		/**@private */
		this._subMeshCount=0;
		/**@private */
		this._boundingBox=null;
		/**@private */
		this._boundingSphere=null;
		/**@private */
		this._boundingBoxCorners=null;
		/**@private 只读,不允许修改。*/
		this._positions=null;
		BaseMesh.__super.call(this);
		this._boundingBoxCorners=__newvec(8,null);
	}

	__class(BaseMesh,'laya.d3.resource.models.BaseMesh',_super);
	var __proto=BaseMesh.prototype;
	/**
	*获取网格顶点,请重载此方法。
	*@return 网格顶点。
	*/
	__proto._getPositions=function(){
		throw new Error("未Override,请重载该属性！");
	}

	/**
	*@private
	*/
	__proto._generateBoundingObject=function(){
		this._boundingSphere=new BoundSphere(new Vector3(),0);
		BoundSphere.createfromPoints(this._positions,this._boundingSphere);
		this._boundingBox=new BoundBox(new Vector3(),new Vector3());
		BoundBox.createfromPoints(this._positions,this._boundingBox);
		this._boundingBox.getCorners(this._boundingBoxCorners);
	}

	/**
	*获取渲染单元数量,请重载此方法。
	*@return 渲染单元数量。
	*/
	__proto.getRenderElementsCount=function(){
		throw new Error("未Override,请重载该属性！");
	}

	/**
	*获取渲染单元,请重载此方法。
	*@param index 索引。
	*@return 渲染单元。
	*/
	__proto.getRenderElement=function(index){
		throw new Error("未Override,请重载该属性！");
	}

	/**
	*获取SubMesh的个数。
	*@return SubMesh的个数。
	*/
	__getset(0,__proto,'subMeshCount',function(){
		return this._subMeshCount;
	});

	/**
	*获取AABB包围盒,禁止修改其数据。
	*@return AABB包围盒。
	*/
	__getset(0,__proto,'boundingBox',function(){
		return this._boundingBox;
	});

	/**
	*获取包围球顶点,禁止修改其数据。
	*@return 包围球。
	*/
	__getset(0,__proto,'boundingBoxCorners',function(){
		return this._boundingBoxCorners;
	});

	/**
	*获取包围球,禁止修改其数据。
	*@return 包围球。
	*/
	__getset(0,__proto,'boundingSphere',function(){
		return this._boundingSphere;
	});

	return BaseMesh;
})(Resource)



/**
*<code>MeshFilter</code> 类用于创建网格过滤器。
*/
//class laya.d3.core.MeshFilter extends laya.d3.core.GeometryFilter
var MeshFilter=(function(_super){
	function MeshFilter(owner){
		/**@private */
		this._owner=null;
		/**@private */
		this._sharedMesh=null;
		MeshFilter.__super.call(this);
		this._owner=owner;
	}

	__class(MeshFilter,'laya.d3.core.MeshFilter',_super);
	var __proto=MeshFilter.prototype;
	/**
	*@private
	*/
	__proto._sharedMeshLoaded=function(){
		this.event(/*laya.events.Event.LOADED*/"loaded");
	}

	/**
	*@inheritDoc
	*/
	__proto._destroy=function(){
		_super.prototype._destroy.call(this);
		this._owner=null;
		(this._sharedMesh)&& (this._sharedMesh._removeReference(),this._sharedMesh=null);
	}

	/**
	*设置共享网格。
	*@return value 共享网格。
	*/
	/**
	*获取共享网格。
	*@return 共享网格。
	*/
	__getset(0,__proto,'sharedMesh',function(){
		return this._sharedMesh;
		},function(value){
		var lastValue=this._sharedMesh;
		(lastValue)&& (lastValue._removeReference());
		this._sharedMesh=value;
		value._addReference();
		this.event(/*laya.events.Event.MESH_CHANGED*/"meshchanged",[this,lastValue,value]);
		if (!value.loaded){
			this._sharedMesh.once(/*laya.events.Event.LOADED*/"loaded",this,this._sharedMeshLoaded);
		}
	});

	/**
	*@inheritDoc
	*/
	__getset(0,__proto,'_isAsyncLoaded',function(){
		return this._sharedMesh.loaded;
	});

	/**
	*@inheritDoc
	*/
	__getset(0,__proto,'_originalBoundingBoxCorners',function(){
		return this._sharedMesh.boundingBoxCorners;
	});

	/**
	*@inheritDoc
	*/
	__getset(0,__proto,'_originalBoundingSphere',function(){
		return this._sharedMesh.boundingSphere;
	});

	/**
	*@inheritDoc
	*/
	__getset(0,__proto,'_originalBoundingBox',function(){
		return this._sharedMesh.boundingBox;
	});

	return MeshFilter;
})(GeometryFilter)


/**
*<code>MeshRender</code> 类用于网格渲染器。
*/
//class laya.d3.core.MeshRender extends laya.d3.core.render.BaseRender
var MeshRender=(function(_super){
	/**
	*创建一个新的 <code>MeshRender</code> 实例。
	*/
	function MeshRender(owner){
		MeshRender.__super.call(this,owner);
		(owner).meshFilter.on(/*laya.events.Event.MESH_CHANGED*/"meshchanged",this,this._onMeshChanged);
	}

	__class(MeshRender,'laya.d3.core.MeshRender',_super);
	var __proto=MeshRender.prototype;
	/**
	*@private
	*/
	__proto._onMeshChanged=function(meshFilter,oldMesh,mesh){
		if (mesh.loaded){
			this._boundingSphereNeedChange=this._boundingBoxNeedChange=this._boundingBoxCenterNeedChange=this._octreeNodeNeedChange=true;
			}else {
			mesh.once(/*laya.events.Event.LOADED*/"loaded",this,this._onMeshLoaed);
		}
	}

	/**
	*@private
	*/
	__proto._onMeshLoaed=function(sender,enable){
		this._boundingSphereNeedChange=this._boundingBoxNeedChange=this._boundingBoxCenterNeedChange=this._octreeNodeNeedChange=true;
	}

	/**
	*@private
	*/
	__proto._calculateBoundingSphereByInitSphere=function(boundSphere){
		var maxScale=NaN;
		var transform=this._owner.transform;
		var scaleE=transform.scale.elements;
		var scaleX=Math.abs(scaleE[0]);
		var scaleY=Math.abs(scaleE[1]);
		var scaleZ=Math.abs(scaleE[2]);
		if (scaleX >=scaleY && scaleX >=scaleZ)
			maxScale=scaleX;
		else
		maxScale=scaleY >=scaleZ ? scaleY :scaleZ;
		Vector3.transformCoordinate(boundSphere.center,transform.worldMatrix,this._boundingSphere.center);
		this._boundingSphere.radius=boundSphere.radius *maxScale;
	}

	/**
	*@private
	*/
	__proto._calculateBoundBoxByInitCorners=function(corners){
		var worldMat=(this._owner).transform.worldMatrix;
		for (var i=0;i < 8;i++)
		Vector3.transformCoordinate(corners[i],worldMat,BaseRender._tempBoundBoxCorners[i]);
		BoundBox.createfromPoints(BaseRender._tempBoundBoxCorners,this._boundingBox);
	}

	/**
	*@inheritDoc
	*/
	__proto._calculateBoundingSphere=function(){
		var sharedMesh=(this._owner).meshFilter.sharedMesh;
		if (sharedMesh==null || sharedMesh.boundingSphere==null)
			this._boundingSphere.toDefault();
		else
		this._calculateBoundingSphereByInitSphere(sharedMesh.boundingSphere);
	}

	/**
	*@inheritDoc
	*/
	__proto._calculateBoundingBox=function(){
		var sharedMesh=(this._owner).meshFilter.sharedMesh;
		if (sharedMesh==null || sharedMesh.boundingBox==null)
			this._boundingBox.toDefault();
		else
		this._calculateBoundBoxByInitCorners(sharedMesh.boundingBoxCorners);
	}

	/**
	*@private
	*/
	__proto._renderUpdate=function(projectionView){
		var transform=this._owner.transform;
		if (transform){
			this._setShaderValueMatrix4x4(/*laya.d3.core.Sprite3D.WORLDMATRIX*/0,transform.worldMatrix);
			var projViewWorld=this._owner.getProjectionViewWorldMatrix(projectionView);
			this._setShaderValueMatrix4x4(/*laya.d3.core.Sprite3D.MVPMATRIX*/1,projViewWorld);
			}else {
			this._setShaderValueMatrix4x4(/*laya.d3.core.Sprite3D.WORLDMATRIX*/0,Matrix4x4.DEFAULT);
			this._setShaderValueMatrix4x4(/*laya.d3.core.Sprite3D.MVPMATRIX*/1,projectionView);
		}
		if (Laya3D.debugMode)
			this._renderRenderableBoundBox();
		return true;
	}

	return MeshRender;
})(BaseRender)


/**
*<code>BaseScene</code> 类用于实现场景。
*/
//class laya.d3.core.scene.Scene extends laya.display.Sprite
var Scene=(function(_super){
	function Scene(){
		this._time=0;
		/**@private */
		//this.__loaded=false;
		/**@private */
		//this._url=null;
		/**@private */
		//this._group=null;
		/**@private */
		//this._lightmaps=null;
		/**@private */
		this._enableLightCount=3;
		/**@private */
		//this._renderTargetTexture=null;
		/**@private */
		this._customRenderQueneIndex=11;
		/**@private */
		//this._lastCurrentTime=NaN;
		/**@private */
		//this._enableFog=false;
		/**@private */
		//this._enableDepthFog=false;
		/**@private */
		//this._fogStart=NaN;
		/**@private */
		//this._fogRange=NaN;
		/**@private */
		//this._fogColor=null;
		/**@private */
		//this._ambientColor=null;
		/**@private */
		//this._shaderValues=null;
		/**@private */
		//this._shaderDefineValue=0;
		/**@private */
		//this._cullingRendersLength=0;
		/**@private */
		//this._cullingRenders=null;
		/**@private */
		/**是否启用灯光。*/
		this.enableLight=true;
		/**四/八叉树的根节点。*/
		//this.treeRoot=null;
		/**四/八叉树的尺寸。*/
		//this.treeSize=null;
		/**四/八叉树的层数。*/
		//this.treeLevel=0;
		//阴影相关变量
		//this.parallelSplitShadowMaps=null;
		/**@private */
		//this._componentsMap=null;
		/**@private */
		//this._typeComponentsIndices=null;
		/**@private */
		//this._components=null;
		Scene.__super.call(this);
		this._renderState=new RenderState();
		this._lights=[];
		this._quenes=[];
		this._cameraPool=[];
		this._renderableSprite3Ds=[];
		this.__loaded=true;
		this._lightmaps=[];
		this._shaderValues=new ValusArray();
		this.parallelSplitShadowMaps=[];
		this._cullingRenders=[];
		this._cullingRendersLength=0;
		this.enableFog=false;
		this.fogStart=300;
		this.fogRange=1000;
		this.fogColor=new Vector3(0.7,0.7,0.7);
		this.ambientColor=new Vector3(0.212,0.227,0.259);
		(WebGL.shaderHighPrecision)&& (this.addShaderDefine(ShaderCompile3D.SHADERDEFINE_HIGHPRECISION));
		this.on(/*laya.events.Event.DISPLAY*/"display",this,this._display);
		this.on(/*laya.events.Event.UNDISPLAY*/"undisplay",this,this._unDisplay);
		this._componentsMap=[];
		this._typeComponentsIndices=[];
		this._components=[];
	}

	__class(Scene,'laya.d3.core.scene.Scene',_super);
	var __proto=Scene.prototype;
	Laya.imps(__proto,{"laya.webgl.submit.ISubmit":true,"laya.resource.ICreateResource":true})
	/**
	*@private
	*/
	__proto._setUrl=function(url){
		this._url=url;
	}

	/**
	*@private
	*/
	__proto._getGroup=function(){
		return this._group;
	}

	/**
	*@private
	*/
	__proto._setGroup=function(value){
		this._group=value;
	}

	/**
	*@private
	*/
	__proto._display=function(){
		Laya.stage._scenes.push(this);
		Laya.stage._scenes.sort(Scene._sortScenes);
		for (var i=0,n=this._childs.length;i < n;i++){
			var spr=this._childs[i];
			(spr.active)&& (spr._activeHierarchy());
		}
	}

	/**
	*@private
	*/
	__proto._unDisplay=function(){
		var scenes=Laya.stage._scenes;
		scenes.splice(scenes.indexOf(this),1);
		for (var i=0,n=this._childs.length;i < n;i++){
			var spr=this._childs[i];
			(spr.active)&& (spr._inActiveHierarchy());
		}
	}

	/**
	*@private
	*/
	__proto._addChild3D=function(sprite3D){
		sprite3D.transform._onWorldTransform();
		sprite3D._setBelongScene(this);
		(this.displayedInStage && sprite3D.active)&& (sprite3D._activeHierarchy());
	}

	/**
	*@private
	*/
	__proto._removeChild3D=function(sprite3D){
		sprite3D.transform.parent=null;
		(this.displayedInStage && sprite3D.active)&& (sprite3D._inActiveHierarchy());
		sprite3D._setUnBelongScene();
	}

	/**
	*@private
	*场景相关渲染准备设置。
	*@param gl WebGL上下文。
	*@return state 渲染状态。
	*/
	__proto._prepareUpdateToRenderState=function(gl,state){
		state.elapsedTime=this._lastCurrentTime ? this.timer.currTimer-this._lastCurrentTime :0;
		this._lastCurrentTime=this.timer.currTimer;
		state.scene=this;
	}

	/**
	*@private
	*/
	__proto._prepareSceneToRender=function(state){
		var lightCount=this._lights.length;
		if (lightCount > 0){
			var renderLightCount=0;
			for (var i=0;i < lightCount;i++){
				if (!this._lights[i]._prepareToScene(state))
					continue ;
				renderLightCount++;
				if (renderLightCount >=this._enableLightCount)
					break ;
			}
		}
	}

	/**
	*@private
	*/
	__proto._updateChilds=function(state){
		for (var i=0,n=this._childs.length;i < n;++i)
		this._childs[i]._update(state);
	}

	/**
	*@private
	*/
	__proto._preRenderScene=function(gl,state,boundFrustum){
		var view=state._viewMatrix;
		var projection=state._projectionMatrix;
		var projectionView=state._projectionViewMatrix;
		var i=0,iNum=0;
		var camera=state.camera;
		if (camera.useOcclusionCulling){
			if (this.treeRoot)
				FrustumCulling.renderObjectCullingOctree(boundFrustum,this,camera,view,projection,projectionView);
			else
			FrustumCulling.renderObjectCulling(boundFrustum,this,camera,view,projection,projectionView);
			}else {
			FrustumCulling.renderObjectCullingNoBoundFrustum(this,camera,view,projection,projectionView);
		}
		for (i=0,iNum=this._quenes.length;i < iNum;i++)
		(this._quenes[i])&& (this._quenes[i]._preRender(state));
	}

	/**
	*@private
	*/
	__proto._clear=function(gl,state){
		var viewport=state._viewport;
		var camera=state.camera;
		var vpX=viewport.x;
		var vpY=camera.renderTargetSize.height-viewport.y-viewport.height;
		var vpWidth=viewport.width;
		var vpHeight=viewport.height;
		gl.viewport(vpX,vpY,vpWidth,vpHeight);
		var flag=/*laya.webgl.WebGLContext.DEPTH_BUFFER_BIT*/0x00000100;
		var renderTarget=camera.renderTarget;
		switch (camera.clearFlag){
			case /*laya.d3.core.BaseCamera.CLEARFLAG_SOLIDCOLOR*/0:;
				var clearColor=camera.clearColor;
				if (clearColor){
					gl.enable(/*laya.webgl.WebGLContext.SCISSOR_TEST*/0x0C11);
					gl.scissor(vpX,vpY,vpWidth,vpHeight);
					var clearColorE=clearColor.elements;
					gl.clearColor(clearColorE[0],clearColorE[1],clearColorE[2],clearColorE[3]);
					flag |=/*laya.webgl.WebGLContext.COLOR_BUFFER_BIT*/0x00004000;
				}
				if (renderTarget){
					(clearColor)|| (flag=/*laya.webgl.WebGLContext.COLOR_BUFFER_BIT*/0x00004000);
				switch (renderTarget.depthStencilFormat){
					case /*laya.webgl.WebGLContext.DEPTH_COMPONENT16*/0x81A5:
						flag |=/*laya.webgl.WebGLContext.DEPTH_BUFFER_BIT*/0x00000100;
						break ;
					case /*laya.webgl.WebGLContext.STENCIL_INDEX8*/0x8D48:
						flag |=/*laya.webgl.WebGLContext.STENCIL_BUFFER_BIT*/0x00000400;
						break ;
					case /*laya.webgl.WebGLContext.DEPTH_STENCIL*/0x84F9:
						flag |=/*laya.webgl.WebGLContext.DEPTH_BUFFER_BIT*/0x00000100;
						flag |=/*laya.webgl.WebGLContext.STENCIL_BUFFER_BIT*/0x00000400;
						break ;
					}
			}
			gl.clear(flag);
			if (clearColor)
				gl.disable(/*laya.webgl.WebGLContext.SCISSOR_TEST*/0x0C11);
			break ;
			case /*laya.d3.core.BaseCamera.CLEARFLAG_SKY*/1:
			case /*laya.d3.core.BaseCamera.CLEARFLAG_DEPTHONLY*/2:
			if (renderTarget){
				flag=/*laya.webgl.WebGLContext.COLOR_BUFFER_BIT*/0x00004000;
				switch (renderTarget.depthStencilFormat){
					case /*laya.webgl.WebGLContext.DEPTH_COMPONENT16*/0x81A5:
						flag |=/*laya.webgl.WebGLContext.DEPTH_BUFFER_BIT*/0x00000100;
						break ;
					case /*laya.webgl.WebGLContext.STENCIL_INDEX8*/0x8D48:
						flag |=/*laya.webgl.WebGLContext.STENCIL_BUFFER_BIT*/0x00000400;
						break ;
					case /*laya.webgl.WebGLContext.DEPTH_STENCIL*/0x84F9:
						flag |=/*laya.webgl.WebGLContext.DEPTH_BUFFER_BIT*/0x00000100;
						flag |=/*laya.webgl.WebGLContext.STENCIL_BUFFER_BIT*/0x00000400
						break ;
					}
			}
			gl.clear(flag);
			break ;
			case /*laya.d3.core.BaseCamera.CLEARFLAG_NONE*/3:
			break ;
			default :
			throw new Error("BaseScene:camera clearFlag invalid.");
		}
	}

	/**
	*@private
	*/
	__proto._renderScene=function(gl,state){
		var camera=state.camera;
		var i=0,n=0;
		var queue;
		for (i=0;i < 2;i++){
			queue=this._quenes[i];
			if (queue){
				camera.renderTarget ? queue._render(state,true):queue._render(state,false);
			}
		}
		if (camera.clearFlag===/*laya.d3.core.BaseCamera.CLEARFLAG_SKY*/1){
			var sky=camera.sky;
			if (sky){
				WebGLContext.setCullFace(gl,false);
				WebGLContext.setDepthFunc(gl,/*laya.webgl.WebGLContext.LEQUAL*/0x0203);
				WebGLContext.setDepthMask(gl,false);
				sky._render(state);
				WebGLContext.setDepthFunc(gl,/*laya.webgl.WebGLContext.LESS*/0x0201);
				WebGLContext.setDepthMask(gl,true);
			}
		}
		for (i=2,n=this._quenes.length;i < n;i++){
			queue=this._quenes[i];
			if (queue){
				queue._sortAlpha(state.camera.transform.position);
				camera.renderTarget ? queue._render(state,true):queue._render(state,false);
			}
		}
	}

	/**
	*@private
	*/
	__proto._set3DRenderConfig=function(gl){
		gl.disable(/*laya.webgl.WebGLContext.BLEND*/0x0BE2);
		WebGLContext._blend=false;
		gl.blendFunc(/*laya.webgl.WebGLContext.SRC_ALPHA*/0x0302,/*laya.webgl.WebGLContext.ONE_MINUS_SRC_ALPHA*/0x0303);
		WebGLContext._sFactor=/*laya.webgl.WebGLContext.SRC_ALPHA*/0x0302;
		WebGLContext._dFactor=/*laya.webgl.WebGLContext.ONE_MINUS_SRC_ALPHA*/0x0303;
		gl.disable(/*laya.webgl.WebGLContext.DEPTH_TEST*/0x0B71);
		WebGLContext._depthTest=false;
		gl.enable(/*laya.webgl.WebGLContext.CULL_FACE*/0x0B44);
		WebGLContext._cullFace=true;
		gl.depthMask(1);
		WebGLContext._depthMask=true;
		gl.frontFace(/*laya.webgl.WebGLContext.CW*/0x0900);
		WebGLContext._frontFace=/*laya.webgl.WebGLContext.CW*/0x0900;
	}

	/**
	*@private
	*/
	__proto._set2DRenderConfig=function(gl){
		WebGLContext.setBlend(gl,true);
		WebGLContext.setBlendFunc(gl,/*laya.webgl.WebGLContext.ONE*/1,/*laya.webgl.WebGLContext.ONE_MINUS_SRC_ALPHA*/0x0303);
		WebGLContext.setDepthTest(gl,false);
		WebGLContext.setCullFace(gl,false);
		WebGLContext.setDepthMask(gl,true);
		WebGLContext.setFrontFace(gl,/*laya.webgl.WebGLContext.CCW*/0x0901);
		gl.viewport(0,0,RenderState2D.width,RenderState2D.height);
	}

	/**
	*@private
	*/
	__proto._parseCustomProps=function(rootNode,innerResouMap,customProps,nodeData){
		var lightMapsData=nodeData.customProps.lightmaps;
		var lightMapCount=lightMapsData.length;
		var lightmaps=this._lightmaps;
		lightmaps.length=lightMapCount;
		for (var i=0;i < lightMapCount;i++)
		lightmaps[i]=Loader.getRes(innerResouMap[lightMapsData[i].replace(".exr",".png")]);
		this.setlightmaps(lightmaps);
		var ambientColorData=nodeData.customProps.ambientColor;
		(ambientColorData)&& (this.ambientColor=new Vector3(ambientColorData[0],ambientColorData[1],ambientColorData[2]));
		var fogColorData=nodeData.customProps.fogColor;
		(fogColorData)&& (this.fogColor=new Vector3(fogColorData[0],fogColorData[1],fogColorData[2]));
	}

	/**
	*@private
	*/
	__proto._addLight=function(light){
		if (this._lights.indexOf(light)< 0)this._lights.push(light);
	}

	/**
	*@private
	*/
	__proto._removeLight=function(light){
		var index=this._lights.indexOf(light);
		index >=0 && (this._lights.splice(index,1));
	}

	/**
	*@private
	*/
	__proto._updateScene=function(){
		var renderState=this._renderState;
		this._prepareUpdateToRenderState(WebGL.mainContext,renderState);
		this._updateComponents(renderState);
		this._updateChilds(renderState);
		this._lateUpdateComponents(renderState);
		this._time+=renderState.elapsedTime / 1000;
		this._shaderValues.setValue(/*CLASS CONST:laya.d3.core.scene.Scene.TIME*/22,this._time);
	}

	/**
	*@private
	*/
	__proto._updateSceneConch=function(){
		var renderState=this._renderState;
		this._prepareUpdateToRenderState(WebGL.mainContext,renderState);
		this._updateComponents(renderState);
		this._lateUpdateComponents(renderState);
		this._prepareSceneToRender(renderState);
		for (var i=0,n=this._cameraPool.length;i < n;i++){
			var camera=this._cameraPool[i];
			renderState.camera=camera;
			camera._prepareCameraToRender();
		}
	}

	/**
	*@private
	*/
	__proto._preRenderShadow=function(state,lightFrustum,shdowQueues,lightViewProjectMatrix,nPSSMNum){
		if (this.treeRoot){
			FrustumCulling.renderShadowObjectCullingOctree(this,lightFrustum,shdowQueues,lightViewProjectMatrix,nPSSMNum);
			}else {
			FrustumCulling.renderShadowObjectCulling(this,lightFrustum,shdowQueues,lightViewProjectMatrix,nPSSMNum);
		}
		for (var i=0,iNum=shdowQueues.length;i < iNum;i++)
		(shdowQueues[i])&& (shdowQueues[i]._preRender(state));
	}

	/**
	*@private
	*/
	__proto._renderShadowMap=function(gl,state,sceneCamera){
		var parallelSplitShadowMap=this.parallelSplitShadowMaps[0];
		parallelSplitShadowMap._calcAllLightCameraInfo(sceneCamera);
		var pssmNum=parallelSplitShadowMap.PSSMNum;
		this._preRenderShadow(state,parallelSplitShadowMap._lightCulling,parallelSplitShadowMap._shadowQuenes,parallelSplitShadowMap._lightVPMatrix[0],pssmNum);
		this.addShaderDefine(ParallelSplitShadowMap.SHADERDEFINE_CAST_SHADOW);
		var renderTarget,shadowQuene,lightCamera;
		if (pssmNum > 1){
			for (var i=0;i < pssmNum;i++){
				renderTarget=parallelSplitShadowMap.getRenderTarget(i+1);
				parallelSplitShadowMap.beginRenderTarget(i+1);
				gl.clearColor(1,1,1,1);
				gl.clear(/*laya.webgl.WebGLContext.COLOR_BUFFER_BIT*/0x00004000 | /*laya.webgl.WebGLContext.DEPTH_BUFFER_BIT*/0x00000100);
				gl.viewport(0,0,renderTarget.width,renderTarget.height);
				state.camera=lightCamera=parallelSplitShadowMap.getLightCamera(i);
				lightCamera._prepareCameraToRender();
				lightCamera._prepareCameraViewProject(lightCamera.viewMatrix,lightCamera.projectionMatrix);
				state._projectionViewMatrix=parallelSplitShadowMap._lightVPMatrix[i+1];
				shadowQuene=parallelSplitShadowMap._shadowQuenes[i];
				shadowQuene._preRender(state);
				shadowQuene._renderShadow(state,false);
				parallelSplitShadowMap.endRenderTarget(i+1);
			}
			}else {
			renderTarget=parallelSplitShadowMap.getRenderTarget(1);
			parallelSplitShadowMap.beginRenderTarget(1);
			gl.clearColor(1,1,1,1);
			gl.clear(/*laya.webgl.WebGLContext.COLOR_BUFFER_BIT*/0x00004000 | /*laya.webgl.WebGLContext.DEPTH_BUFFER_BIT*/0x00000100);
			gl.viewport(0,0,renderTarget.width,renderTarget.height);
			state.camera=lightCamera=parallelSplitShadowMap.getLightCamera(0);
			lightCamera._prepareCameraToRender();
			lightCamera._prepareCameraViewProject(lightCamera.viewMatrix,lightCamera.projectionMatrix);
			state._projectionViewMatrix=parallelSplitShadowMap._lightVPMatrix[0];
			shadowQuene=parallelSplitShadowMap._shadowQuenes[0];
			shadowQuene._preRender(state);
			shadowQuene._renderShadow(state,true);
			parallelSplitShadowMap.endRenderTarget(1);
		}
		this.removeShaderDefine(ParallelSplitShadowMap.SHADERDEFINE_CAST_SHADOW);
	}

	/**
	*@private
	*/
	__proto.addTreeNode=function(renderObj){
		this.treeRoot.addTreeNode(renderObj);
	}

	/**
	*@private
	*/
	__proto.removeTreeNode=function(renderObj){
		if (!this.treeSize)return;
		if (renderObj._treeNode){
			renderObj._treeNode.removeObject(renderObj);
		}
	}

	/**
	*设置光照贴图。
	*@param value 光照贴图。
	*/
	__proto.setlightmaps=function(value){
		this._lightmaps=value;
		for (var i=0,n=this._renderableSprite3Ds.length;i < n;i++)
		this._renderableSprite3Ds[i]._render._applyLightMapParams();
	}

	/**
	*获取光照贴图。
	*@return 获取光照贴图。
	*/
	__proto.getlightmaps=function(){
		return this._lightmaps;
	}

	/**
	*@inheritDoc
	*/
	__proto.addChildAt=function(node,index){
		if (!((node instanceof laya.d3.core.Sprite3D )))
			throw new Error("Sprite3D:Node type must Sprite3D.");
		if (!node || this.destroyed || node===this)return node;
		if ((node).zOrder)this._set$P("hasZorder",true);
		if (index >=0 && index <=this._childs.length){
			if (node._parent===this){
				var oldIndex=this.getChildIndex(node);
				this._childs.splice(oldIndex,1);
				this._childs.splice(index,0,node);
				if (this.conchModel){
					this.conchModel.removeChild(node.conchModel);
					this.conchModel.addChildAt(node.conchModel,index);
				}
				this._childChanged();
				}else {
				node.parent && node.parent.removeChild(node);
				this._childs===Node.ARRAY_EMPTY && (this._childs=[]);
				this._childs.splice(index,0,node);
				this.conchModel && this.conchModel.addChildAt(node.conchModel,index);
				node.parent=this;
				this._addChild3D(node);
			}
			return node;
			}else {
			throw new Error("appendChildAt:The index is out of bounds");
		}
	}

	/**
	*@inheritDoc
	*/
	__proto.addChild=function(node){
		if (!((node instanceof laya.d3.core.Sprite3D )))
			throw new Error("Sprite3D:Node type must Sprite3D.");
		if (!node || this.destroyed || node===this)return node;
		if ((node).zOrder)this._set$P("hasZorder",true);
		if (node._parent===this){
			var index=this.getChildIndex(node);
			if (index!==this._childs.length-1){
				this._childs.splice(index,1);
				this._childs.push(node);
				if (this.conchModel){
					this.conchModel.removeChild(node.conchModel);
					this.conchModel.addChildAt(node.conchModel,this._childs.length-1);
				}
				this._childChanged();
			}
			}else {
			node.parent && node.parent.removeChild(node);
			this._childs===Node.ARRAY_EMPTY && (this._childs=[]);
			this._childs.push(node);
			this.conchModel && this.conchModel.addChildAt(node.conchModel,this._childs.length-1);
			node.parent=this;
			this._childChanged();
			this._addChild3D(node);
		}
		return node;
	}

	/**
	*@inheritDoc
	*/
	__proto.removeChildAt=function(index){
		var node=this.getChildAt(index);
		if (node){
			this._removeChild3D(node);
			this._childs.splice(index,1);
			this.conchModel && this.conchModel.removeChild(node.conchModel);
			node.parent=null;
		}
		return node;
	}

	/**
	*@inheritDoc
	*/
	__proto.removeChildren=function(beginIndex,endIndex){
		(beginIndex===void 0)&& (beginIndex=0);
		(endIndex===void 0)&& (endIndex=0x7fffffff);
		if (this._childs && this._childs.length > 0){
			var childs=this._childs;
			if (beginIndex===0 && endIndex >=n){
				var arr=childs;
				this._childs=Node.ARRAY_EMPTY;
				}else {
				arr=childs.splice(beginIndex,endIndex-beginIndex);
			}
			for (var i=0,n=arr.length;i < n;i++){
				this._removeChild3D(arr [i]);
				arr[i].parent=null;
				this.conchModel && this.conchModel.removeChild(arr[i].conchModel);
			}
		}
		return this;
	}

	/**
	*@inheritDoc
	*/
	__proto.addFrustumCullingObject=function(renderObject){
		if (this.treeRoot){
			this.addTreeNode(renderObject);
			}else {
			if (this._cullingRendersLength===this._cullingRenders.length)
				this._cullingRenders.push(renderObject);
			else
			this._cullingRenders[this._cullingRendersLength]=renderObject;
			renderObject._indexInSceneFrustumCullingObjects=this._cullingRendersLength++;
		}
	}

	/**
	*@private
	*/
	__proto.removeFrustumCullingObject=function(renderObject){
		if (this.treeRoot){
			this.removeTreeNode(renderObject);
			}else {
			this._cullingRendersLength--;
			var indexInSceneFrustumCullingObjects=renderObject._indexInSceneFrustumCullingObjects;
			if (indexInSceneFrustumCullingObjects!==this._cullingRendersLength){
				var endRender=this._cullingRenders[this._cullingRendersLength];
				this._cullingRenders[indexInSceneFrustumCullingObjects]=endRender;
				endRender._indexInSceneFrustumCullingObjects=indexInSceneFrustumCullingObjects;
				renderObject._indexInSceneFrustumCullingObjects=-1;
			}
		}
	}

	/**
	*获得某个渲染队列。
	*@param index 渲染队列索引。
	*@return 渲染队列。
	*/
	__proto.getRenderQueue=function(index){
		return this._quenes[index] || (this._quenes[index]=new RenderQueue(this));
	}

	/**
	*添加渲染队列。
	*@param renderConfig 渲染队列配置文件。
	*/
	__proto.addRenderQuene=function(){
		this._quenes[this._customRenderQueneIndex++]=new RenderQueue(this);
	}

	/**
	*增加shader宏定义。
	*@param define shader宏定义。
	*/
	__proto.addShaderDefine=function(define){
		this._shaderDefineValue |=define;
	}

	/**
	*移除shader宏定义。
	*@param define shader宏定义。
	*/
	__proto.removeShaderDefine=function(define){
		this._shaderDefineValue &=~define;
	}

	/**
	*添加指定类型脚本。
	*@param type 脚本类型。
	*@return 组件。
	*/
	__proto.addScript=function(type){
		return this._addComponent(type);
	}

	/**
	*通过指定类型和类型索引获得脚本。
	*@param type 脚本类型。
	*@param typeIndex 脚本索引。
	*@return 脚本。
	*/
	__proto.getScriptByType=function(type,typeIndex){
		(typeIndex===void 0)&& (typeIndex=0);
		return this._getComponentByType(type,typeIndex);
	}

	/**
	*通过指定类型获得所有脚本。
	*@param type 脚本类型。
	*@param scripts 脚本输出队列。
	*/
	__proto.getScriptsByType=function(type,scripts){
		this._getComponentsByType(type,scripts);
	}

	/**
	*通过指定索引获得脚本。
	*@param index 索引。
	*@return 脚本。
	*/
	__proto.getScriptByIndex=function(index){
		return this._getComponentByIndex(index);
	}

	/**
	*通过指定类型和类型索引移除脚本。
	*@param type 脚本类型。
	*@param typeIndex 类型索引。
	*/
	__proto.removeScriptByType=function(type,typeIndex){
		(typeIndex===void 0)&& (typeIndex=0);
		this._removeComponentByType(type,typeIndex);
	}

	/**
	*通过指定类型移除所有脚本。
	*@param type 组件类型。
	*/
	__proto.removeScriptsByType=function(type){
		this._removeComponentByType(type);
	}

	/**
	*移除全部脚本。
	*/
	__proto.removeAllScript=function(){
		this._removeAllComponent();
	}

	/**
	*@private
	*/
	__proto.render=function(context,x,y){
		(Render._context.ctx)._renderKey=0;
		this._childs.length > 0 && context.addRenderObject(this);
	}

	/**
	*@private
	*/
	__proto.renderSubmit=function(){
		var gl=WebGL.mainContext;
		var renderState=this._renderState;
		this._set3DRenderConfig(gl);
		this._prepareSceneToRender(this._renderState);
		var i=0,n=0,camera;
		for (i=0,n=this._cameraPool.length;i < n;i++){
			camera=this._cameraPool[i];
			(camera.activeInHierarchy)&& (camera._renderCamera(gl,renderState,this));		
		}
		this._set2DRenderConfig(gl);
		return 1;
	}

	/**
	*@private
	*/
	__proto.onAsynLoaded=function(url,data,params){
		var json=data[0]
		if (json.type!=="Scene")
			throw new Error("Scene: the .lh file root type must be Scene,please use other function to  load  this file.");
		var innerResouMap=data[1];
		Utils3D._createNodeByJson(this,json,this,innerResouMap);
		this.event(/*laya.events.Event.HIERARCHY_LOADED*/"hierarchyloaded",[this]);
		this.__loaded=true;
	}

	/**
	*@private
	*/
	__proto.destroy=function(destroyChild){
		(destroyChild===void 0)&& (destroyChild=true);
		if (this.destroyed)
			return;
		_super.prototype.destroy.call(this,destroyChild);
		this._renderState=null;
		this._lights=null;
		this._lightmaps=null;
		this._renderTargetTexture=null;
		this._shaderValues=null;
		this._cullingRenders=null;
		this._quenes=null;
		this._cameraPool=null;
		this._renderableSprite3Ds=null;
		this.treeRoot=null;
		this.treeSize=null;
		this.parallelSplitShadowMaps=null;
		this._typeComponentsIndices=null;
		this._components=null;
		Loader.clearRes(this.url);
		(this.loaded)||(Laya3D._cancelLoadByUrl(this.url));
	}

	/**
	*@private
	*/
	__proto.getRenderType=function(){
		return 0;
	}

	/**
	*@private
	*/
	__proto.releaseRender=function(){}
	/**
	*@private
	*/
	__proto._addComponent=function(type){
		var typeComponentIndex;
		var index=this._componentsMap.indexOf(type);
		if (index===-1){
			typeComponentIndex=[];
			this._componentsMap.push(type);
			this._typeComponentsIndices.push(typeComponentIndex);
			}else {
			typeComponentIndex=this._typeComponentsIndices[index];
			if (this._components[typeComponentIndex[0]].isSingleton)
				throw new Error("无法单实例创建"+type+"组件"+"，"+type+"组件已存在！");
		};
		var component=ClassUtils.getInstance(type);
		typeComponentIndex.push(this._components.length);
		this._components.push(component);
		var _this=this;
		component._initialize(_this);
		return component;
	}

	/**
	*@private
	*/
	__proto._removeComponent=function(mapIndex,index){
		var componentIndices=this._typeComponentsIndices[mapIndex];
		var componentIndex=componentIndices[index];
		var component=this._components[componentIndex];
		this._components.splice(componentIndex,1);
		componentIndices.splice(index,1);
		(componentIndices.length===0)&& (this._typeComponentsIndices.splice(mapIndex,1),this._componentsMap.splice(mapIndex,1));
		for (var i=0,n=this._componentsMap.length;i < n;i++){
			componentIndices=this._typeComponentsIndices[i];
			for (var j=componentIndices.length-1;j >=0;j--){
				var oldComponentIndex=componentIndices[j];
				if (oldComponentIndex > componentIndex)
					componentIndices[j]=--oldComponentIndex;
				else
				break ;
			}
		}
		component._destroy();
	}

	/**
	*@private
	*/
	__proto._getComponentByType=function(type,typeIndex){
		(typeIndex===void 0)&& (typeIndex=0);
		var mapIndex=this._componentsMap.indexOf(type);
		if (mapIndex===-1)
			return null;
		return this._components[this._typeComponentsIndices[mapIndex][typeIndex]];
	}

	/**
	*@private
	*/
	__proto._getComponentsByType=function(type,components){
		var index=this._componentsMap.indexOf(type);
		if (index===-1){
			components.length=0;
			return;
		};
		var typeComponents=this._typeComponentsIndices[index];
		var count=typeComponents.length;
		components.length=count;
		for (var i=0;i < count;i++)
		components[i]=this._components[typeComponents[i]];
	}

	/**
	*@private
	*/
	__proto._getComponentByIndex=function(index){
		return this._components[index];
	}

	/**
	*@private
	*/
	__proto._removeComponentByType=function(type,typeIndex){
		(typeIndex===void 0)&& (typeIndex=0);
		var mapIndex=this._componentsMap.indexOf(type);
		if (mapIndex===-1)
			return;
		this._removeComponent(mapIndex,typeIndex);
	}

	/**
	*@private
	*/
	__proto._removeComponentsByType=function(type){
		var mapIndex=this._componentsMap.indexOf(type);
		if (mapIndex===-1)
			return;
		var componentIndices=this._typeComponentsIndices[mapIndex];
		for (var i=0,n=componentIndices.length;i < n;componentIndices.length < n ? n--:i++)
		this._removeComponent(mapIndex,i);
	}

	/**
	*@private
	*/
	__proto._removeAllComponent=function(){
		for (var i=0,n=this._componentsMap.length;i < n;this._componentsMap.length < n ? n--:i++)
		this._removeComponentsByType(this._componentsMap[i]);
	}

	/**
	*@private
	*/
	__proto._updateComponents=function(state){
		for (var i=0,n=this._components.length;i < n;i++){
			var component=this._components[i];
			(!component.started)&& (component._start(state),component.started=true);
			(component.enable)&& (component._update(state));
		}
	}

	/**
	*@private
	*/
	__proto._lateUpdateComponents=function(state){
		for (var i=0;i < this._components.length;i++){
			var component=this._components[i];
			(!component.started)&& (component._start(state),component.started=true);
			(component.enable)&& (component._lateUpdate(state));
		}
	}

	/**
	*@private
	*/
	__proto._preRenderUpdateComponents=function(state){
		for (var i=0;i < this._components.length;i++){
			var component=this._components[i];
			(!component.started)&& (component._start(state),component.started=true);
			(component.enable)&& (component._preRenderUpdate(state));
		}
	}

	/**
	*@private
	*/
	__proto._postRenderUpdateComponents=function(state){
		for (var i=0;i < this._components.length;i++){
			var component=this._components[i];
			(!component.started)&& (component._start(state),component.started=true);
			(component.enable)&& (component._postRenderUpdate(state));
		}
	}

	/**
	*@private
	*/
	__getset(0,__proto,'_loaded',null,function(value){
		this.__loaded=value;
	});

	/**
	*设置雾化颜色。
	*@param value 雾化颜色。
	*/
	/**
	*获取雾化颜色。
	*@return 雾化颜色。
	*/
	__getset(0,__proto,'fogColor',function(){
		return this._fogColor;
		},function(value){
		this._fogColor=value;
		this._shaderValues.setValue(/*CLASS CONST:laya.d3.core.scene.Scene.FOGCOLOR*/0,value.elements);
	});

	/**
	*设置是否允许雾化。
	*@param value 是否允许雾化。
	*/
	/**
	*获取是否允许雾化。
	*@return 是否允许雾化。
	*/
	__getset(0,__proto,'enableFog',function(){
		return this._enableFog;
		},function(value){
		if (this._enableFog!==value){
			this._enableFog=value;
			if (value){
				this.addShaderDefine(ShaderCompile3D.SHADERDEFINE_FOG);
				this.removeShaderDefine(ShaderCompile3D.SAHDERDEFINE_DEPTHFOG);
			}else
			this.removeShaderDefine(ShaderCompile3D.SHADERDEFINE_FOG);
		}
	});

	/**
	*获取资源的URL地址。
	*@return URL地址。
	*/
	__getset(0,__proto,'url',function(){
		return this._url;
	});

	/**
	*获取是否已加载完成。
	*/
	__getset(0,__proto,'loaded',function(){
		return this.__loaded;
	});

	__getset(0,__proto,'enableDepthFog',function(){
		return this._enableDepthFog;
		},function(v){
		if (this._enableDepthFog !=v){
			this._enableDepthFog=v;
			if (v){
				this.addShaderDefine(ShaderCompile3D.SAHDERDEFINE_DEPTHFOG);
				this.removeShaderDefine(ShaderCompile3D.SHADERDEFINE_FOG);
				}else {
				this.removeShaderDefine(ShaderCompile3D.SAHDERDEFINE_DEPTHFOG);
			}
		}
	});

	/**
	*设置雾化起始位置。
	*@param value 雾化起始位置。
	*/
	/**
	*获取雾化起始位置。
	*@return 雾化起始位置。
	*/
	__getset(0,__proto,'fogStart',function(){
		return this._fogStart;
		},function(value){
		this._fogStart=value;
		this._shaderValues.setValue(/*CLASS CONST:laya.d3.core.scene.Scene.FOGSTART*/1,value);
	});

	/**
	*设置雾化范围。
	*@param value 雾化范围。
	*/
	/**
	*获取雾化范围。
	*@return 雾化范围。
	*/
	__getset(0,__proto,'fogRange',function(){
		return this._fogRange;
		},function(value){
		this._fogRange=value;
		this._shaderValues.setValue(/*CLASS CONST:laya.d3.core.scene.Scene.FOGRANGE*/2,value);
	});

	/**
	*设置环境光颜色。
	*@param value 环境光颜色。
	*/
	/**
	*获取环境光颜色。
	*@return 环境光颜色。
	*/
	__getset(0,__proto,'ambientColor',function(){
		return this._ambientColor;
		},function(value){
		this._ambientColor=value;
		this._shaderValues.setValue(/*CLASS CONST:laya.d3.core.scene.Scene.AMBIENTCOLOR*/21,value.elements);
	});

	/**
	*获取当前场景。
	*@return 当前场景。
	*/
	__getset(0,__proto,'scene',function(){
		return this;
	});

	/**
	*获取场景的可渲染精灵。
	*/
	__getset(0,__proto,'renderableSprite3Ds',function(){
		return this._renderableSprite3Ds.slice();
	});

	Scene._sortScenes=function(a,b){
		if (a.parent===Laya.stage && b.parent===Laya.stage){
			var stageChildren=Laya.stage._childs;
			return stageChildren.indexOf(a)-stageChildren.indexOf(b);
			}else if (a.parent!==Laya.stage && b.parent!==Laya.stage){
			return Scene._sortScenes(a.parent,b.parent);
			}else {
			return (a.parent===Laya.stage)?-1 :1;
		}
	}

	Scene.load=function(url){
		return Laya.loader.create(url,null,null,Scene);
	}

	Scene.FOGCOLOR=0;
	Scene.FOGSTART=1;
	Scene.FOGRANGE=2;
	Scene.LIGHTDIRECTION=3;
	Scene.LIGHTDIRCOLOR=4;
	Scene.POINTLIGHTPOS=5;
	Scene.POINTLIGHTRANGE=6;
	Scene.POINTLIGHTATTENUATION=7;
	Scene.POINTLIGHTCOLOR=8;
	Scene.SPOTLIGHTPOS=9;
	Scene.SPOTLIGHTDIRECTION=10;
	Scene.SPOTLIGHTSPOT=11;
	Scene.SPOTLIGHTRANGE=12;
	Scene.SPOTLIGHTATTENUATION=13;
	Scene.SPOTLIGHTCOLOR=14;
	Scene.SHADOWDISTANCE=15;
	Scene.SHADOWLIGHTVIEWPROJECT=16;
	Scene.SHADOWMAPPCFOFFSET=17;
	Scene.SHADOWMAPTEXTURE1=18;
	Scene.SHADOWMAPTEXTURE2=19;
	Scene.SHADOWMAPTEXTURE3=20;
	Scene.AMBIENTCOLOR=21;
	Scene.TIME=22;
	return Scene;
})(Sprite)


/**
*<code>Sprite3D</code> 类用于实现3D精灵。
*/
//class laya.d3.core.Sprite3D extends laya.d3.core.ComponentNode
var Sprite3D=(function(_super){
	function Sprite3D(name){
		/**@private */
		//this._projectionViewWorldMatrix=null;
		/**@private */
		//this._projectionViewWorldUpdateLoopCount=0;
		/**@private */
		//this._projectionViewWorldUpdateCamera=null;
		/**@private */
		//this._id=0;
		/**@private */
		//this.__loaded=false;
		/**@private */
		//this._url=null;
		/**@private */
		//this._group=null;
		/**@private */
		//this._active=false;
		/**@private */
		//this._activeInHierarchy=false;
		/**@private */
		//this._layer=null;
		/**@private */
		//this._shaderDefineValue=0;
		/**@private */
		//this._shaderValues=null;
		/**@private */
		//this._colliders=null;
		/**@private */
		//this._scene=null;
		/**@private */
		//this._transform=null;
		/**@private */
		//this._hierarchyAnimator=null;
		/**是否静态,静态包含一系列的静态处理。*/
		//this.isStatic=false;
		Sprite3D.__super.call(this);
		this.__loaded=true;
		this._projectionViewWorldUpdateLoopCount=-1;
		this._activeInHierarchy=false;
		this._projectionViewWorldMatrix=new Matrix4x4();
		this._shaderValues=new ValusArray();
		this._colliders=[];
		this._id=++Sprite3D._uniqueIDCounter;
		this._transform=new Transform3D(this);
		this.name=name ? name :"Sprite3D-"+Sprite3D._nameNumberCounter++;
		this.layer=Layer.currentCreationLayer;
		this.active=true;
	}

	__class(Sprite3D,'laya.d3.core.Sprite3D',_super);
	var __proto=Sprite3D.prototype;
	Laya.imps(__proto,{"laya.d3.core.render.IUpdate":true,"laya.resource.ICreateResource":true,"laya.d3.core.IClone":true})
	/**
	*@private
	*/
	__proto._setUrl=function(url){
		this._url=url;
	}

	/**
	*@private
	*/
	__proto._getGroup=function(){
		return this._group;
	}

	/**
	*@private
	*/
	__proto._setGroup=function(value){
		this._group=value;
	}

	/**
	*@private
	*/
	__proto._addChild3D=function(sprite3D){
		sprite3D.transform.parent=this.transform;
		if (this._hierarchyAnimator){
			(!sprite3D._hierarchyAnimator)&& (sprite3D._setHierarchyAnimator(this._hierarchyAnimator,null));
			this._getAnimatorToLinkSprite3D(sprite3D,true,/*new vector.<>*/[sprite3D.name]);
		}
		if (this._scene){
			sprite3D._setBelongScene(this._scene);
			(this._activeInHierarchy && sprite3D._active)&& (sprite3D._activeHierarchy());
		}
	}

	/**
	*@private
	*/
	__proto._removeChild3D=function(sprite3D){
		sprite3D.transform.parent=null;
		if (this._scene){
			(this._activeInHierarchy && sprite3D._active)&& (sprite3D._inActiveHierarchy());
			sprite3D._setUnBelongScene();
		}
		if (this._hierarchyAnimator){
			((sprite3D._hierarchyAnimator==this._hierarchyAnimator))&& (sprite3D._clearHierarchyAnimator(this._hierarchyAnimator,null));
			this._getAnimatorToLinkSprite3D(sprite3D,false,/*new vector.<>*/[sprite3D.name]);
		}
	}

	/**
	*@private
	*/
	__proto._parseBaseCustomProps=function(customProps){
		var loccalPosition=this.transform.localPosition;
		loccalPosition.fromArray(customProps.translate);
		this.transform.localPosition=loccalPosition;
		var localRotation=this.transform.localRotation;
		localRotation.fromArray(customProps.rotation);
		this.transform.localRotation=localRotation;
		var localScale=this.transform.localScale;
		localScale.fromArray(customProps.scale);
		this.transform.localScale=localScale;
		var layerData=customProps.layer;
		(layerData !=null)&& (this.layer=Layer.getLayerByNumber(layerData));
	}

	/**
	*@private
	*/
	__proto._parseCustomComponent=function(rootNode,innerResouMap,componentsData){
		for (var k in componentsData){
			var component=componentsData[k];
			switch (k){
				case "Animator":;
					var animator=this.addComponent(Animator);
					if (component.avatarPath){
						animator.avatar=Loader.getRes(innerResouMap[component.avatarPath]);
						}else {
						var avatarData=component.avatar;
						if (avatarData){
							animator.avatar=Loader.getRes(innerResouMap[avatarData.path]);
							var linkSprites=avatarData.linkSprites;
							(linkSprites)&& (rootNode.once(/*laya.events.Event.HIERARCHY_LOADED*/"hierarchyloaded",this,this._onRootNodeHierarchyLoaded,[animator,linkSprites]));
						}
					};
					var clipPaths=component.clipPaths;
					var clipCount=clipPaths.length;
					for (var i=0;i < clipCount;i++)
					animator.addClip(Loader.getRes(innerResouMap[clipPaths[i]]));
					animator.clip=Loader.getRes(innerResouMap[clipPaths[0]]);
					var playOnWake=component.playOnWake;
					(playOnWake!==undefined)&& (animator.playOnWake=playOnWake);
					break ;
				case "Rigidbody":;
					var rigidbody=this.addComponent(Rigidbody);
					break ;
				case "SphereCollider":;
					var sphereCollider=this.addComponent(SphereCollider);
					sphereCollider.isTrigger=component.isTrigger;
					var center=sphereCollider.center;
					center.fromArray(component.center);
					sphereCollider.center=center;
					sphereCollider.radius=component.radius;
					break ;
				case "BoxCollider":;
					var boxCollider=this.addComponent(BoxCollider);
					boxCollider.isTrigger=component.isTrigger;
					boxCollider.center.fromArray(component.center);
					var size=boxCollider.size;
					size.fromArray(component.size);
					boxCollider.size=size;
					break ;
				case "MeshCollider":;
					var meshCollider=this.addComponent(MeshCollider);
					break ;
				default :
				}
		}
	}

	/**
	*@private
	*/
	__proto._onRootNodeHierarchyLoaded=function(animator,linkSprites){
		for (var k in linkSprites){
			var nodeOwner=this;
			var path=linkSprites[k];
			for (var j=0,m=path.length;j < m;j++){
				var p=path[j];
				if (p===""){
					break ;
					}else {
					nodeOwner=nodeOwner.getChildByName(p);
					if (!nodeOwner)
						break ;
				}
			}
			(nodeOwner)&& (animator.linkSprite3DToAvatarNode(k,nodeOwner));
		}
	}

	/**
	*@private
	*/
	__proto._setHierarchyAnimator=function(animator,parentAnimator){
		this._changeHierarchyAnimator(animator);
		for (var i=0,n=this._childs.length;i < n;i++){
			var child=this._childs[i];
			(child._hierarchyAnimator==parentAnimator)&& (child._setHierarchyAnimator(animator,parentAnimator));
		}
	}

	/**
	*@private
	*/
	__proto._clearHierarchyAnimator=function(animator,parentAnimator){
		this._changeHierarchyAnimator(parentAnimator);
		for (var i=0,n=this._childs.length;i < n;i++){
			var child=this._childs[i];
			(child._hierarchyAnimator==animator)&& (child._clearHierarchyAnimator(animator,parentAnimator));
		}
	}

	/**
	*@private
	*/
	__proto._getAnimatorToLinkSprite3D=function(sprite3D,isLink,path){
		var animator=this.getComponentByType(Animator);
		if (animator){
			if (animator.avatar){
				(animator.avatar._version)|| (sprite3D._setAnimatorToLinkAvatar(animator,isLink));
				}else {
				sprite3D._setAnimatorToLinkSprite3DNoAvatar(animator,isLink,path);
			}
		}
		if (this._parent && (this._parent instanceof laya.d3.core.Sprite3D )){
			path.unshift(this._parent.name);
			var p=this._parent;
			(p._hierarchyAnimator)&& (p._getAnimatorToLinkSprite3D(sprite3D,isLink,path));
		}
	}

	/**
	*@private
	*/
	__proto._setAnimatorToLinkSprite3DNoAvatar=function(animator,isLink,path){
		var i=0,n=0;
		for (i=0,n=animator.getClipCount();i < n;i++)
		animator._handleSpriteOwnersBySprite(i,isLink,path,this);
		for (i=0,n=this._childs.length;i < n;i++){
			var child=this._childs[i];
			var index=path.length;
			path.push(child.name);
			child._setAnimatorToLinkSprite3DNoAvatar(animator,isLink,path);
			path.splice(index,1);
		}
	}

	/**
	*@private
	*/
	__proto._changeHierarchyAnimator=function(animator){
		this._hierarchyAnimator=animator;
	}

	/**
	*@private
	*/
	__proto._isLinkSpriteToAnimationNode=function(animator,node,isLink){
		var nodeIndex=animator._avatarNodes.indexOf(node);
		var cacheSpriteToNodesMap=animator._cacheSpriteToNodesMap;
		if (isLink){
			this._transform.dummy=node.transform;
			animator._cacheNodesToSpriteMap[nodeIndex]=cacheSpriteToNodesMap.length;
			cacheSpriteToNodesMap.push(nodeIndex);
			}else {
			this._transform.dummy=null;
			var index=animator._cacheNodesToSpriteMap[nodeIndex];
			animator._cacheNodesToSpriteMap[nodeIndex]=null;
			cacheSpriteToNodesMap.splice(index,1);
		}
	}

	/**
	*@private
	*/
	__proto._setBelongScene=function(scene){
		this._scene=scene;
		for (var i=0,n=this._childs.length;i < n;i++)
		(this._childs [i])._setBelongScene(scene);
	}

	/**
	*@private
	*/
	__proto._setUnBelongScene=function(){
		this._scene=null;
		for (var i=0,n=this._childs.length;i < n;i++)
		(this._childs [i])._setUnBelongScene();
	}

	/**
	*@private
	*/
	__proto._activeHierarchy=function(){
		var i=0,n=0;
		this._activeInHierarchy=true;
		this._addSelfRenderObjects();
		for (i=0,n=this._colliders.length;i < n;i++)
		this._layer._addCollider(this._colliders[i]);
		this.event(/*laya.events.Event.ACTIVE_IN_HIERARCHY_CHANGED*/"activeinhierarchychanged",true);
		for (i=0,n=this._childs.length;i < n;i++){
			var child=this._childs [i];
			(child._active)&& (child._activeHierarchy());
		}
	}

	/**
	*@private
	*/
	__proto._inActiveHierarchy=function(){
		var i=0,n=0;
		this._activeInHierarchy=false;
		this._clearSelfRenderObjects();
		for (i=0,n=this._colliders.length;i < n;i++){
			var col=this._colliders[i];
			col._clearCollsionMap();
			this._layer._removeCollider(col);
		}
		this.event(/*laya.events.Event.ACTIVE_IN_HIERARCHY_CHANGED*/"activeinhierarchychanged",false);
		for (i=0,n=this._childs.length;i < n;i++){
			var child=this._childs [i];
			(child._active)&& (child._inActiveHierarchy());
		}
	}

	/**
	*@private
	*/
	__proto.addComponent=function(type){
		var typeComponentIndex;
		var index=this._componentsMap.indexOf(type);
		if (index===-1){
			typeComponentIndex=[];
			this._componentsMap.push(type);
			this._typeComponentsIndices.push(typeComponentIndex);
			}else {
			typeComponentIndex=this._typeComponentsIndices[index];
			if (this._components[typeComponentIndex[0]].isSingleton)
				throw new Error("无法单实例创建"+type+"组件"+"，"+type+"组件已存在！");
		};
		var component=ClassUtils.getInstance(type);
		typeComponentIndex.push(this._components.length);
		this._components.push(component);
		if ((component instanceof laya.d3.component.physics.Collider )){
			var rigidbody=this.getComponentByType(Rigidbody);
			(rigidbody)&& ((component)._isRigidbody=true);
			(this._activeInHierarchy)&& (this._layer._addCollider(component));
			this._colliders.push(component);
			}else if ((component instanceof laya.d3.component.Animator )){
			var animator=component;
			this._setHierarchyAnimator(animator,this._parent ? (this._parent)._hierarchyAnimator :null);
			this._setAnimatorToLinkSprite3DNoAvatar(animator,true,/*new vector.<>*/[]);
			}else if ((component instanceof laya.d3.component.Rigidbody )){
			for (var i=0,n=this._colliders.length;i < n;i++)
			this._colliders[i]._setIsRigidbody(true);
		}
		if ((component instanceof laya.d3.component.Script ))
			this._scripts.push(component);
		component._initialize(this);
		return component;
	}

	/**
	*@inheritDoc
	*/
	__proto._removeComponent=function(mapIndex,index){
		var i=0,n=0;
		var componentIndices=this._typeComponentsIndices[mapIndex];
		var componentIndex=componentIndices[index];
		var component=this._components[componentIndex];
		if ((component instanceof laya.d3.component.physics.Collider )){
			var colliderComponent=component;
			(this._activeInHierarchy)&& (this._layer._removeCollider(colliderComponent));
			this._colliders.splice(this._colliders.indexOf(colliderComponent),1);
			}else if ((component instanceof laya.d3.component.Animator )){
			var animator=component;
			this._clearHierarchyAnimator(animator,this._parent ? (this._parent)._hierarchyAnimator :null);
			}else if ((component instanceof laya.d3.component.Rigidbody )){
			for (i=0,n=this._colliders.length;i < n;i++){
				var collider=this._colliders[i];
				collider._setIsRigidbody(false);
				var runtimeCollisonMap=collider._runtimeCollisonMap;
				var runtimeCollisonTestMap=collider._runtimeCollisonTestMap;
				for (var k in runtimeCollisonMap)
				delete runtimeCollisonTestMap[k];
			}
		}
		this._components.splice(componentIndex,1);
		if ((component instanceof laya.d3.component.Script ))
			this._scripts.splice(this._scripts.indexOf(component),1);
		componentIndices.splice(index,1);
		(componentIndices.length===0)&& (this._typeComponentsIndices.splice(mapIndex,1),this._componentsMap.splice(mapIndex,1));
		for (i=0,n=this._componentsMap.length;i < n;i++){
			componentIndices=this._typeComponentsIndices[i];
			for (var j=componentIndices.length-1;j >=0;j--){
				var oldComponentIndex=componentIndices[j];
				if (oldComponentIndex > componentIndex)
					componentIndices[j]=--oldComponentIndex;
				else
				break ;
			}
		}
		component._destroy();
	}

	/**
	*清理自身渲染物体，请重载此函数。
	*/
	__proto._clearSelfRenderObjects=function(){}
	/**
	*添加自身渲染物体，请重载此函数。
	*/
	__proto._addSelfRenderObjects=function(){}
	/**
	*@private
	*/
	__proto._parseCustomProps=function(rootNode,innerResouMap,customProps,nodeData){}
	/**
	*更新子节点。
	*@param state 渲染相关状态。
	*/
	__proto._updateChilds=function(state){
		var n=this._childs.length;
		if (n===0)return;
		for (var i=0;i < n;++i)
		this._childs[i]._update((state));
	}

	/**
	*排序函数。
	*@param state 渲染相关状态。
	*/
	__proto._getSortID=function(renderElement,material){
		return material.id */*laya.d3.graphics.VertexDeclaration._maxVertexDeclarationBit*/1000+renderElement._getVertexBuffer().vertexDeclaration.id;
	}

	/**
	*更新
	*@param state 渲染相关状态
	*/
	__proto._update=function(state){
		state.owner=this;
		if (this._activeInHierarchy){
			this._updateComponents(state);
			this._lateUpdateComponents(state);
			Stat.spriteCount++;
			this._childs.length && this._updateChilds(state);
		}
	}

	/**
	*获取投影视图世界矩阵。
	*@param projectionViewMatrix 投影视图矩阵。
	*@return 投影视图世界矩阵。
	*/
	__proto.getProjectionViewWorldMatrix=function(projectionViewMatrix){
		Matrix4x4.multiply(projectionViewMatrix,this.transform.worldMatrix,this._projectionViewWorldMatrix);
		return this._projectionViewWorldMatrix;
	}

	/**
	*加载层级文件，并作为该节点的子节点。
	*@param url
	*/
	__proto.loadHierarchy=function(url){
		this.addChild(laya.d3.core.Sprite3D.load(url));
	}

	/**
	*@inheritDoc
	*/
	__proto.addChildAt=function(node,index){
		if (!((node instanceof laya.d3.core.Sprite3D )))
			throw new Error("Sprite3D:Node type must Sprite3D.");
		if (!node || this.destroyed || node===this)return node;
		if ((node).zOrder)this._set$P("hasZorder",true);
		if (index >=0 && index <=this._childs.length){
			if (node._parent===this){
				var oldIndex=this.getChildIndex(node);
				this._childs.splice(oldIndex,1);
				this._childs.splice(index,0,node);
				if (this.conchModel){
					this.conchModel.removeChild(node.conchModel);
					this.conchModel.addChildAt(node.conchModel,index);
				}
				this._childChanged();
				}else {
				node.parent && node.parent.removeChild(node);
				this._childs===Node.ARRAY_EMPTY && (this._childs=[]);
				this._childs.splice(index,0,node);
				this.conchModel && this.conchModel.addChildAt(node.conchModel,index);
				node.parent=this;
				this._addChild3D(node);
			}
			return node;
			}else {
			throw new Error("appendChildAt:The index is out of bounds");
		}
	}

	/**
	*@inheritDoc
	*/
	__proto.addChild=function(node){
		if (!((node instanceof laya.d3.core.Sprite3D )))
			throw new Error("Sprite3D:Node type must Sprite3D.");
		if (!node || this.destroyed || node===this)return node;
		if ((node).zOrder)this._set$P("hasZorder",true);
		if (node._parent===this){
			var index=this.getChildIndex(node);
			if (index!==this._childs.length-1){
				this._childs.splice(index,1);
				this._childs.push(node);
				if (this.conchModel){
					this.conchModel.removeChild(node.conchModel);
					this.conchModel.addChildAt(node.conchModel,this._childs.length-1);
				}
				this._childChanged();
			}
			}else {
			node.parent && node.parent.removeChild(node);
			this._childs===Node.ARRAY_EMPTY && (this._childs=[]);
			this._childs.push(node);
			this.conchModel && this.conchModel.addChildAt(node.conchModel,this._childs.length-1);
			node.parent=this;
			this._childChanged();
			this._addChild3D(node);
		}
		return node;
	}

	/**
	*@inheritDoc
	*/
	__proto.removeChildAt=function(index){
		var node=this.getChildAt(index);
		if (node){
			this._removeChild3D(node);
			this._childs.splice(index,1);
			this.conchModel && this.conchModel.removeChild(node.conchModel);
			node.parent=null;
		}
		return node;
	}

	/**
	*@inheritDoc
	*/
	__proto.removeChildren=function(beginIndex,endIndex){
		(beginIndex===void 0)&& (beginIndex=0);
		(endIndex===void 0)&& (endIndex=0x7fffffff);
		if (this._childs && this._childs.length > 0){
			var childs=this._childs;
			if (beginIndex===0 && endIndex >=n){
				var arr=childs;
				this._childs=Node.ARRAY_EMPTY;
				}else {
				arr=childs.splice(beginIndex,endIndex-beginIndex);
			}
			for (var i=0,n=arr.length;i < n;i++){
				this._removeChild3D(arr [i]);
				arr[i].parent=null;
				this.conchModel && this.conchModel.removeChild(arr[i].conchModel);
			}
		}
		return this;
	}

	/**
	*@private
	*/
	__proto.onAsynLoaded=function(url,data,params){
		var json=data[0];
		if (json.type!=="Sprite3D")
			throw new Error("Sprite3D: The .lh file root type must be Sprite3D,please use other function to  load  this file.");
		var innerResouMap=data[1];
		Utils3D._createNodeByJson(this,json,this,innerResouMap);
		this.event(/*laya.events.Event.HIERARCHY_LOADED*/"hierarchyloaded",[this]);
		this.__loaded=true;
	}

	/**
	*克隆。
	*@param destObject 克隆源。
	*/
	__proto.cloneTo=function(destObject){
		if (this.destroyed)
			throw new Error("Sprite3D: Can't be cloned if the Sprite3D has destroyed.");
		var destSprite3D=destObject;
		destSprite3D.name=this.name;
		destSprite3D._destroyed=this._destroyed;
		destSprite3D.timer=this.timer;
		destSprite3D._$P=this._$P;
		destSprite3D.active=this._active;
		var destLocalPosition=destSprite3D.transform.localPosition;
		var destPosition=destSprite3D.transform.position;
		this.transform.localPosition.cloneTo(destLocalPosition);
		this.transform.position.cloneTo(destPosition);
		destSprite3D.transform.localPosition=destLocalPosition;
		destSprite3D.transform.position=destPosition;
		var destLocalRotation=destSprite3D.transform.localRotation;
		this.transform.localRotation.cloneTo(destLocalRotation);
		destSprite3D.transform.localRotation=destLocalRotation;
		var destLocalScale=destSprite3D.transform.localScale;
		this.transform.localScale.cloneTo(destLocalScale);
		destSprite3D.transform.localScale=destLocalScale;
		destSprite3D.isStatic=this.isStatic;
		var i=0,n=0;
		for (i=0,n=this._componentsMap.length;i < n;i++){
			var destComponent=destSprite3D.addComponent(this._componentsMap[i]);
			this._components[i]._cloneTo(destComponent);
		}
		for (i=0,n=this._childs.length;i < n;i++)
		destSprite3D.addChild(this._childs[i].clone());
	}

	/**
	*克隆。
	*@return 克隆副本。
	*/
	__proto.clone=function(){
		var destSprite3D=/*__JS__ */new this.constructor();
		this.cloneTo(destSprite3D);
		return destSprite3D;
	}

	/**
	*@inheritDoc
	*/
	__proto.destroy=function(destroyChild){
		(destroyChild===void 0)&& (destroyChild=true);
		if (this.destroyed)
			return;
		laya.display.Node.prototype.destroy.call(this,destroyChild);
		var i=0,n=0;
		for (i=0,n=this._components.length;i < n;i++)
		this._components[i]._destroy();
		this._components=null;
		this._componentsMap=null;
		this._typeComponentsIndices=null;
		this._transform=null;
		this._colliders=null;
		Loader.clearRes(this.url);
		(this.loaded)|| (Laya3D._cancelLoadByUrl(this.url));
	}

	/**
	*@private
	*/
	__proto._handleSpriteToAvatar=function(animator,isLink){
		var i=0,n=0;
		var avatarNodes=animator._avatarNodes;
		var node=animator._avatarNodeMap[this.name];
		if (node && node.name===this.name && !this._transform.dummy)
			this._isLinkSpriteToAnimationNode(animator,node,isLink);
	}

	/**
	*@private
	*/
	__proto._setAnimatorToLinkAvatar=function(animator,isLink){
		this._handleSpriteToAvatar(animator,isLink);
		for (var i=0,n=this._childs.length;i < n;i++){
			var child=this._childs[i];
			child._setAnimatorToLinkAvatar(animator,isLink);
		}
	}

	/**
	*获取在场景中是否激活。
	*@return 在场景中是否激活。
	*/
	__getset(0,__proto,'activeInHierarchy',function(){
		return this._activeInHierarchy;
	});

	/**
	*@private
	*/
	__getset(0,__proto,'_loaded',null,function(value){
		this.__loaded=value;
	});

	/**
	*设置是否激活。
	*@param value 是否激活。
	*/
	/**
	*获取自身是否激活。
	*@return 自身是否激活。
	*/
	__getset(0,__proto,'active',function(){
		return this._active;
		},function(value){
		if (this._active!==value){
			this._active=value;
			if (this._parent){
				if ((this._parent===this._scene && this._parent.displayedInStage)|| (this._parent)._activeInHierarchy){
					if (value)
						this._activeHierarchy();
					else
					this._inActiveHierarchy();
				}
			}
		}
	});

	/**
	*获得组件的数量。
	*@return 组件数量。
	*/
	__getset(0,__proto,'componentsCount',function(){
		return this._components.length;
	});

	/**
	*获取是否已加载完成。
	*/
	__getset(0,__proto,'loaded',function(){
		return this.__loaded;
	});

	/**
	*获取唯一标识ID。
	*@return 唯一标识ID。
	*/
	__getset(0,__proto,'id',function(){
		return this._id;
	});

	/**
	*获取资源的URL地址。
	*@return URL地址。
	*/
	__getset(0,__proto,'url',function(){
		return this._url;
	});

	/**
	*设置蒙版。
	*@param value 蒙版。
	*/
	/**
	*获取蒙版。
	*@return 蒙版。
	*/
	__getset(0,__proto,'layer',function(){
		return this._layer;
		},function(value){
		if (this._layer!==value){
			if (value){
				if (this._activeInHierarchy){
					var i=0,n=this._colliders.length;
					if (this._layer){
						for (i=0;i < n;i++)
						this._layer._removeCollider(this._colliders[i]);
					}
					for (i=0;i < n;i++)
					value._addCollider(this._colliders[i]);
				}
				this._layer=value;
				this.event(/*laya.events.Event.LAYER_CHANGED*/"layerchanged",value);
				}else {
				throw new Error("Layer value can be null.");
			}
		}
	});

	/**
	*获得所属场景。
	*@return 场景。
	*/
	__getset(0,__proto,'scene',function(){
		return this._scene;
	});

	/**
	*获取精灵变换。
	*/
	__getset(0,__proto,'transform',function(){
		return this._transform;
	});

	Sprite3D.instantiate=function(original,parent,worldPositionStays,position,rotation){
		(worldPositionStays===void 0)&& (worldPositionStays=true);
		var destSprite3D=original.clone();
		(parent)&& (parent.addChild(destSprite3D));
		var transform=destSprite3D.transform;
		if (worldPositionStays){
			var worldMatrix=transform.worldMatrix;
			original.transform.worldMatrix.cloneTo(worldMatrix);
			transform.worldMatrix=worldMatrix;
			}else {
			(position)&& (transform.position=position);
			(rotation)&& (transform.rotation=rotation);
		}
		return destSprite3D;
	}

	Sprite3D.load=function(url){
		return Laya.loader.create(url,null,null,Sprite3D);
	}

	Sprite3D.WORLDMATRIX=0;
	Sprite3D.MVPMATRIX=1;
	Sprite3D._uniqueIDCounter=0;
	Sprite3D._nameNumberCounter=0;
	return Sprite3D;
})(ComponentNode)



//class laya.d3.shader.Shader3D extends laya.webgl.shader.BaseShader
var Shader3D=(function(_super){
	function Shader3D(vs,ps,attributeMap,sceneUniformMap,cameraUniformMap,spriteUniformMap,materialUniformMap,renderElementUniformMap){
		//this._attributeMap=null;
		//this._sceneUniformMap=null;
		//this._cameraUniformMap=null;
		//this._spriteUniformMap=null;
		//this._materialUniformMap=null;
		//this._renderElementUniformMap=null;
		//this._vs=null;
		//this._ps=null;
		this._curActTexIndex=0;
		//this._reCompile=false;
		//this._vshader=null;
		//this._pshader=null;
		this._program=null;
		this._attributeParams=null;
		this._uniformParams=null;
		this._attributeParamsMap=[];
		this._sceneUniformParamsMap=[];
		this._cameraUniformParamsMap=[];
		this._spriteUniformParamsMap=[];
		this._materialUniformParamsMap=[];
		this._renderElementUniformParamsMap=[];
		//this._id=0;
		/**@private */
		//this._uploadLoopCount=0;
		/**@private */
		//this._uploadRenderElement=null;
		/**@private */
		//this._uploadMaterial=null;
		/**@private */
		//this._uploadSprite3D=null;
		/**@private */
		//this._uploadCamera=null;
		/**@private */
		//this._uploadScene=null;
		/**@private */
		//this._uploadVertexBuffer=null;
		Shader3D.__super.call(this);
		if ((!vs)|| (!ps))throw "Shader Error";
		this._id=++Shader3D._count;
		this._vs=vs;
		this._ps=ps;
		this._attributeMap=attributeMap;
		this._sceneUniformMap=sceneUniformMap;
		this._cameraUniformMap=cameraUniformMap;
		this._spriteUniformMap=spriteUniformMap;
		this._materialUniformMap=materialUniformMap;
		this._renderElementUniformMap=renderElementUniformMap;
		this.recreateResource();
	}

	__class(Shader3D,'laya.d3.shader.Shader3D',_super);
	var __proto=Shader3D.prototype;
	__proto.recreateResource=function(){
		this._compile();
		this.completeCreate();
		this.memorySize=0;
	}

	//忽略尺寸尺寸
	__proto.disposeResource=function(){
		WebGL.mainContext.deleteShader(this._vshader);
		WebGL.mainContext.deleteShader(this._pshader);
		WebGL.mainContext.deleteProgram(this._program);
		this._vshader=this._pshader=this._program=null;
		this._attributeParams=null;
		this._uniformParams=null;
		this.memorySize=0;
		this._curActTexIndex=0;
	}

	__proto._compile=function(){
		if (!this._vs || !this._ps || this._attributeParams || this._uniformParams)
			return;
		this._reCompile=true;
		this._attributeParams=[];
		this._uniformParams=[];
		var text=[this._vs,this._ps];
		var result;
		var gl=WebGL.mainContext;
		this._program=gl.createProgram();
		this._vshader=Shader3D._createShader(gl,text[0],/*laya.webgl.WebGLContext.VERTEX_SHADER*/0x8B31);
		this._pshader=Shader3D._createShader(gl,text[1],/*laya.webgl.WebGLContext.FRAGMENT_SHADER*/0x8B30);
		gl.attachShader(this._program,this._vshader);
		gl.attachShader(this._program,this._pshader);
		gl.linkProgram(this._program);
		if (!Render.isConchApp && ShaderCompile3D.debugMode && !gl.getProgramParameter(this._program,/*laya.webgl.WebGLContext.LINK_STATUS*/0x8B82))
			throw gl.getProgramInfoLog(this._program);
		var one,i=0,j=0,n=0,location;
		var attribNum=0;
		if (Render.isConchApp){
			attribNum=gl.getProgramParameterEx(this._vs,this._ps,"",/*laya.webgl.WebGLContext.ACTIVE_ATTRIBUTES*/0x8B89);
		}
		else{
			attribNum=gl.getProgramParameter(this._program,/*laya.webgl.WebGLContext.ACTIVE_ATTRIBUTES*/0x8B89);
		}
		for (i=0;i < attribNum;i++){
			var attrib=null;
			if (Render.isConchApp){
				attrib=gl.getActiveAttribEx(this._vs,this._ps,"",i);
			}
			else{
				attrib=gl.getActiveAttrib(this._program,i);
			}
			location=gl.getAttribLocation(this._program,attrib.name);
			one={vartype:"attribute",ivartype:0,attrib:attrib,location:location,name:attrib.name,type:attrib.type,isArray:false,isSame:false,preValue:null,indexOfParams:0};
			this._attributeParams.push(one);
		};
		var nUniformNum=0;
		if (Render.isConchApp){
			nUniformNum=gl.getProgramParameterEx(this._vs,this._ps,"",/*laya.webgl.WebGLContext.ACTIVE_UNIFORMS*/0x8B86);
		}
		else{
			nUniformNum=gl.getProgramParameter(this._program,/*laya.webgl.WebGLContext.ACTIVE_UNIFORMS*/0x8B86);
		}
		for (i=0;i < nUniformNum;i++){
			var uniform=null;
			if (Render.isConchApp){
				uniform=gl.getActiveUniformEx(this._vs,this._ps,"",i);
			}
			else{
				uniform=gl.getActiveUniform(this._program,i);
			}
			location=gl.getUniformLocation(this._program,uniform.name);
			one={vartype:"uniform",ivartype:1,attrib:attrib,location:location,name:uniform.name,type:uniform.type,isArray:false,isSame:false,preValue:null,indexOfParams:0};
			if (one.name.indexOf('[0]')> 0){
				one.name=one.name.substr(0,one.name.length-3);
				one.isArray=true;
				one.location=gl.getUniformLocation(this._program,one.name);
			}
			this._uniformParams.push(one);
		}
		for (i=0,n=this._attributeParams.length;i < n;i++){
			one=this._attributeParams[i];
			one.indexOfParams=i;
			one.index=1;
			one.value=[one.location,null];
			one.codename=one.name;
			one.name=(this._attributeMap[one.codename] !=null)? this._attributeMap[one.codename] :one.codename;
			this._attributeParamsMap.push(one.name);
			this._attributeParamsMap.push(one);
			one._this=this;
			one.uploadedValue=[];
			one.fun=this._attribute;
		}
		for (i=0,n=this._uniformParams.length;i < n;i++){
			one=this._uniformParams[i];
			one.indexOfParams=i;
			one.index=1;
			one.value=[one.location,null];
			one.codename=one.name;
			if (this._sceneUniformMap[one.codename] !=null){
				one.name=this._sceneUniformMap[one.codename];
				this._sceneUniformParamsMap.push(one.name);
				this._sceneUniformParamsMap.push(one);
				}else if (this._cameraUniformMap[one.codename] !=null){
				one.name=this._cameraUniformMap[one.codename];
				this._cameraUniformParamsMap.push(one.name);
				this._cameraUniformParamsMap.push(one);
				}else if (this._spriteUniformMap[one.codename] !=null){
				one.name=this._spriteUniformMap[one.codename];
				this._spriteUniformParamsMap.push(one.name);
				this._spriteUniformParamsMap.push(one);
				}else if (this._materialUniformMap[one.codename] !=null){
				one.name=this._materialUniformMap[one.codename];
				this._materialUniformParamsMap.push(one.name);
				this._materialUniformParamsMap.push(one);
				}else if (this._renderElementUniformMap[one.codename] !=null){
				one.name=this._renderElementUniformMap[one.codename];
				this._renderElementUniformParamsMap.push(one.name);
				this._renderElementUniformParamsMap.push(one);
				}else {
				console.log("Shader:can't find uinform name:"+one.codename+" in shader file.");
			}
			one._this=this;
			one.uploadedValue=[];
			switch (one.type){
				case /*laya.webgl.WebGLContext.INT*/0x1404:
					one.fun=one.isArray ? this._uniform1iv :this._uniform1i;
					break ;
				case /*laya.webgl.WebGLContext.FLOAT*/0x1406:
					one.fun=one.isArray ? this._uniform1fv :this._uniform1f;
					break ;
				case /*laya.webgl.WebGLContext.FLOAT_VEC2*/0x8B50:
					one.fun=one.isArray ? this._uniform_vec2v :this._uniform_vec2;
					break ;
				case /*laya.webgl.WebGLContext.FLOAT_VEC3*/0x8B51:
					one.fun=one.isArray ? this._uniform_vec3v :this._uniform_vec3;
					break ;
				case /*laya.webgl.WebGLContext.FLOAT_VEC4*/0x8B52:
					one.fun=one.isArray ? this._uniform_vec4v :this._uniform_vec4;
					break ;
				case /*laya.webgl.WebGLContext.SAMPLER_2D*/0x8B5E:
					one.fun=this._uniform_sampler2D;
					break ;
				case /*laya.webgl.WebGLContext.SAMPLER_CUBE*/0x8B60:
					one.fun=this._uniform_samplerCube;
					break ;
				case /*laya.webgl.WebGLContext.FLOAT_MAT4*/0x8B5C:
					one.fun=this._uniformMatrix4fv;
					break ;
				case /*laya.webgl.WebGLContext.BOOL*/0x8B56:
					one.fun=this._uniform1i;
					break ;
				case /*laya.webgl.WebGLContext.FLOAT_MAT2*/0x8B5A:
					one.fun=this._uinformMatrix2fv;
					break ;
				case /*laya.webgl.WebGLContext.FLOAT_MAT3*/0x8B5B:
					one.fun=this._uinformMatrix3fv;
					break ;
				default :
					throw new Error("compile shader err!");
					break ;
				}
		}
	}

	__proto._attribute=function(one,value){
		var gl=WebGL.mainContext;
		var enableAtributes=Buffer._enableAtributes;
		var location=one.location;
		(enableAtributes[location])|| (gl.enableVertexAttribArray(location));
		gl.vertexAttribPointer(location,value[0],value[1],value[2],value[3],value[4]);
		enableAtributes[location]=Buffer._bindVertexBuffer;
		return 1;
	}

	__proto._uniform1f=function(one,value){
		var uploadedValue=one.uploadedValue;
		if (uploadedValue[0]!==value){
			WebGL.mainContext.uniform1f(one.location,uploadedValue[0]=value);
			return 1;
		}
		return 0;
	}

	__proto._uniform1fv=function(one,value){
		if (value.length < 4){
			var uploadedValue=one.uploadedValue;
			if (uploadedValue[0]!==value[0] || uploadedValue[1]!==value[1] || uploadedValue[2]!==value[2] || uploadedValue[3]!==value[3]){
				WebGL.mainContext.uniform1fv(one.location,value);
				uploadedValue[0]=value[0];
				uploadedValue[1]=value[1];
				uploadedValue[2]=value[2];
				uploadedValue[3]=value[3];
				return 1;
			}
			return 0;
			}else {
			WebGL.mainContext.uniform1fv(one.location,value);
			return 1;
		}
	}

	__proto._uniform_vec2=function(one,value){
		var uploadedValue=one.uploadedValue;
		if (uploadedValue[0]!==value[0] || uploadedValue[1]!==value[1]){
			WebGL.mainContext.uniform2f(one.location,uploadedValue[0]=value[0],uploadedValue[1]=value[1]);
			return 1;
		}
		return 0;
	}

	__proto._uniform_vec2v=function(one,value){
		if (value.length < 2){
			var uploadedValue=one.uploadedValue;
			if (uploadedValue[0]!==value[0] || uploadedValue[1]!==value[1] || uploadedValue[2]!==value[2] || uploadedValue[3]!==value[3]){
				WebGL.mainContext.uniform2fv(one.location,value);
				uploadedValue[0]=value[0];
				uploadedValue[1]=value[1];
				uploadedValue[2]=value[2];
				uploadedValue[3]=value[3];
				return 1;
			}
			return 0;
			}else {
			WebGL.mainContext.uniform2fv(one.location,value);
			return 1;
		}
	}

	__proto._uniform_vec3=function(one,value){
		var uploadedValue=one.uploadedValue;
		if (uploadedValue[0]!==value[0] || uploadedValue[1]!==value[1] || uploadedValue[2]!==value[2]){
			WebGL.mainContext.uniform3f(one.location,uploadedValue[0]=value[0],uploadedValue[1]=value[1],uploadedValue[2]=value[2]);
			return 1;
		}
		return 0;
	}

	__proto._uniform_vec3v=function(one,value){
		WebGL.mainContext.uniform3fv(one.location,value);
		return 1;
	}

	__proto._uniform_vec4=function(one,value){
		var uploadedValue=one.uploadedValue;
		if (uploadedValue[0]!==value[0] || uploadedValue[1]!==value[1] || uploadedValue[2]!==value[2] || uploadedValue[3]!==value[3]){
			WebGL.mainContext.uniform4f(one.location,uploadedValue[0]=value[0],uploadedValue[1]=value[1],uploadedValue[2]=value[2],uploadedValue[3]=value[3]);
			return 1;
		}
		return 0;
	}

	__proto._uniform_vec4v=function(one,value){
		WebGL.mainContext.uniform4fv(one.location,value);
		return 1;
	}

	__proto._uniformMatrix2fv=function(one,value){
		WebGL.mainContext.uniformMatrix2fv(one.location,false,value);
		return 1;
	}

	__proto._uniformMatrix3fv=function(one,value){
		WebGL.mainContext.uniformMatrix3fv(one.location,false,value);
		return 1;
	}

	__proto._uniformMatrix4fv=function(one,value){
		WebGL.mainContext.uniformMatrix4fv(one.location,false,value);
		return 1;
	}

	__proto._uinformMatrix2fv=function(one,value){
		WebGL.mainContext.uniformMatrix2fv(one.location,false,value);
		return 1;
	}

	__proto._uinformMatrix3fv=function(one,value){
		WebGL.mainContext.uniformMatrix3fv(one.location,false,value);
		return 1;
	}

	__proto._uniform1i=function(one,value){
		var uploadedValue=one.uploadedValue;
		if (uploadedValue[0]!==value){
			WebGL.mainContext.uniform1i(one.location,uploadedValue[0]=value);
			return 1;
		}
		return 0;
	}

	__proto._uniform1iv=function(one,value){
		WebGL.mainContext.uniform1iv(one.location,value);
		return 1;
	}

	__proto._uniform_ivec2=function(one,value){
		var uploadedValue=one.uploadedValue;
		if (uploadedValue[0]!==value[0] || uploadedValue[1]!==value[1]){
			WebGL.mainContext.uniform2i(one.location,uploadedValue[0]=value[0],uploadedValue[1]=value[1]);
			return 1;
		}
		return 0;
	}

	__proto._uniform_ivec2v=function(one,value){
		WebGL.mainContext.uniform2iv(one.location,value);
		return 1;
	}

	__proto._uniform_vec3i=function(one,value){
		var uploadedValue=one.uploadedValue;
		if (uploadedValue[0]!==value[0] || uploadedValue[1]!==value[1] || uploadedValue[2]!==value[2]){
			WebGL.mainContext.uniform3i(one.location,uploadedValue[0]=value[0],uploadedValue[1]=value[1],uploadedValue[2]=value[2]);
			return 1;
		}
		return 0;
	}

	__proto._uniform_vec3vi=function(one,value){
		WebGL.mainContext.uniform3iv(one.location,value);
		return 1;
	}

	__proto._uniform_vec4i=function(one,value){
		var uploadedValue=one.uploadedValue;
		if (uploadedValue[0]!==value[0] || uploadedValue[1]!==value[1] || uploadedValue[2]!==value[2] || uploadedValue[3]!==value[3]){
			WebGL.mainContext.uniform4i(one.location,uploadedValue[0]=value[0],uploadedValue[1]=value[1],uploadedValue[2]=value[2],uploadedValue[3]=value[3]);
			return 1;
		}
		return 0;
	}

	__proto._uniform_vec4vi=function(one,value){
		WebGL.mainContext.uniform4iv(one.location,value);
		return 1;
	}

	__proto._uniform_sampler2D=function(one,texture){
		var value=texture.source || texture.defaulteTexture.source;
		var gl=WebGL.mainContext;
		var uploadedValue=one.uploadedValue;
		if (uploadedValue[0]==null){
			if (this._curActTexIndex > 7)
				throw new Error("Shader3D: shader support textures max count is 8,can't large than it.");
			uploadedValue[0]=this._curActTexIndex;
			gl.uniform1i(one.location,this._curActTexIndex);
			gl.activeTexture(Shader3D._TEXTURES[this._curActTexIndex]);
			if (value)
				WebGLContext.bindTexture(gl,/*laya.webgl.WebGLContext.TEXTURE_2D*/0x0DE1,value);
			this._curActTexIndex++;
			return 1;
			}else {
			gl.activeTexture(Shader3D._TEXTURES[uploadedValue[0]]);
			if (value)
				WebGLContext.bindTexture(gl,/*laya.webgl.WebGLContext.TEXTURE_2D*/0x0DE1,value);
			return 0;
		}
	}

	__proto._uniform_samplerCube=function(one,texture){
		var value=texture.source || texture.defaulteTexture.source;
		var gl=WebGL.mainContext;
		var uploadedValue=one.uploadedValue;
		if (uploadedValue[0]==null){
			if (this._curActTexIndex > 7)
				throw new Error("Shader3D: shader support textures max count is 8,can't large than it.");
			uploadedValue[0]=this._curActTexIndex;
			gl.uniform1i(one.location,this._curActTexIndex);
			gl.activeTexture(Shader3D._TEXTURES[this._curActTexIndex]);
			if (value)
				WebGLContext.bindTexture(gl,/*laya.webgl.WebGLContext.TEXTURE_CUBE_MAP*/0x8513,value);
			else
			WebGLContext.bindTexture(gl,/*laya.webgl.WebGLContext.TEXTURE_CUBE_MAP*/0x8513,SolidColorTextureCube.grayTexture.source);
			this._curActTexIndex++;
			return 1;
			}else {
			gl.activeTexture(Shader3D._TEXTURES[uploadedValue[0]]);
			if (value)
				WebGLContext.bindTexture(gl,/*laya.webgl.WebGLContext.TEXTURE_CUBE_MAP*/0x8513,value);
			else
			WebGLContext.bindTexture(gl,/*laya.webgl.WebGLContext.TEXTURE_CUBE_MAP*/0x8513,SolidColorTextureCube.grayTexture.source);
			return 0;
		}
	}

	__proto._noSetValue=function(one){
		console.log("no....:"+one.name);
	}

	//throw new Error("upload shader err,must set value:"+one.name);
	__proto.bind=function(){
		BaseShader.activeShader=this;
		BaseShader.bindShader=this;
		this.activeResource();
		return WebGLContext.UseProgram(this._program);
	}

	/**
	*按数组的定义提交
	*@param shaderValue 数组格式[name,value,...]
	*/
	__proto.uploadAttributes=function(attributeShaderValue,_bufferUsage){
		var value;
		var one,shaderCall=0;
		for (var i=0,n=this._attributeParamsMap.length;i < n;i+=2){
			one=this._attributeParamsMap[i+1];
			value=attributeShaderValue[this._attributeParamsMap[i]];
			if (value !=null){
				_bufferUsage && _bufferUsage[one.name] && _bufferUsage[one.name].bind();
				shaderCall+=one.fun.call(this,one,value);
			}
		}
		Stat.shaderCall+=shaderCall;
	}

	/**
	*按数组的定义提交
	*@param shaderValue 数组格式[name,value,...]
	*/
	__proto.uploadAttributesX=function(attributeShaderValue,vb){
		var value;
		var one,shaderCall=0;
		for (var i=0,n=this._attributeParamsMap.length;i < n;i+=2){
			one=this._attributeParamsMap[i+1];
			value=attributeShaderValue[this._attributeParamsMap[i]];
			if (value !=null){
				vb._bind();
				shaderCall+=one.fun.call(this,one,value);
			}
		}
		Stat.shaderCall+=shaderCall;
	}

	/**
	*按数组的定义提交
	*@param shaderValue 数组格式[name,value,...]
	*/
	__proto.uploadSceneUniforms=function(shaderValue){
		var value;
		var one,shaderCall=0;
		for (var i=0,n=this._sceneUniformParamsMap.length;i < n;i+=2){
			one=this._sceneUniformParamsMap[i+1];
			value=shaderValue[this._sceneUniformParamsMap[i]];
			if (value !=null)
				shaderCall+=one.fun.call(this,one,value);
		}
		Stat.shaderCall+=shaderCall;
	}

	/**
	*按数组的定义提交
	*@param shaderValue 数组格式[name,value,...]
	*/
	__proto.uploadCameraUniforms=function(shaderValue){
		var value;
		var one,shaderCall=0;
		for (var i=0,n=this._cameraUniformParamsMap.length;i < n;i+=2){
			one=this._cameraUniformParamsMap[i+1];
			value=shaderValue[this._cameraUniformParamsMap[i]];
			if (value !=null)
				shaderCall+=one.fun.call(this,one,value);
		}
		Stat.shaderCall+=shaderCall;
	}

	/**
	*按数组的定义提交
	*@param shaderValue 数组格式[name,value,...]
	*/
	__proto.uploadSpriteUniforms=function(shaderValue){
		var value;
		var one,shaderCall=0;
		for (var i=0,n=this._spriteUniformParamsMap.length;i < n;i+=2){
			one=this._spriteUniformParamsMap[i+1];
			value=shaderValue[this._spriteUniformParamsMap[i]];
			if (value !=null)
				shaderCall+=one.fun.call(this,one,value);
		}
		Stat.shaderCall+=shaderCall;
	}

	/**
	*按数组的定义提交
	*@param shaderValue 数组格式[name,value,...]
	*/
	__proto.uploadMaterialUniforms=function(shaderValue){
		var value;
		var one,shaderCall=0;
		for (var i=0,n=this._materialUniformParamsMap.length;i < n;i+=2){
			one=this._materialUniformParamsMap[i+1];
			value=shaderValue[this._materialUniformParamsMap[i]];
			if (value !=null)
				shaderCall+=one.fun.call(this,one,value);
		}
		Stat.shaderCall+=shaderCall;
	}

	/**
	*按数组的定义提交
	*@param shaderValue 数组格式[name,value,...]
	*/
	__proto.uploadRenderElementUniforms=function(shaderValue){
		var value;
		var one,shaderCall=0;
		for (var i=0,n=this._renderElementUniformParamsMap.length;i < n;i+=2){
			one=this._renderElementUniformParamsMap[i+1];
			value=shaderValue[this._renderElementUniformParamsMap[i]];
			if (value !=null)
				shaderCall+=one.fun.call(this,one,value);
		}
		Stat.shaderCall+=shaderCall;
	}

	Shader3D.create=function(vs,ps,attributeMap,sceneUniformMap,cameraUniformMap,spriteUniformMap,materialUniformMap,renderElementUniformMap){
		return new Shader3D(vs,ps,attributeMap,sceneUniformMap,cameraUniformMap,spriteUniformMap,materialUniformMap,renderElementUniformMap);
	}

	Shader3D.addInclude=function(fileName,txt){
		ShaderCompile.addInclude(fileName,txt);
	}

	Shader3D._createShader=function(gl,str,type){
		var shader=gl.createShader(type);
		gl.shaderSource(shader,str);
		gl.compileShader(shader);
		if (ShaderCompile3D.debugMode && !gl.getShaderParameter(shader,/*laya.webgl.WebGLContext.COMPILE_STATUS*/0x8B81))
		{
			var szError = gl.getShaderInfoLog(shader);
			throw szError;
		}
		return shader;
	}

	Shader3D.PERIOD_RENDERELEMENT=0;
	Shader3D.PERIOD_MATERIAL=1;
	Shader3D.PERIOD_SPRITE=2;
	Shader3D.PERIOD_CAMERA=3;
	Shader3D.PERIOD_SCENE=4;
	Shader3D._TEXTURES=[ /*laya.webgl.WebGLContext.TEXTURE0*/0x84C0,/*laya.webgl.WebGLContext.TEXTURE1*/0x84C1,/*laya.webgl.WebGLContext.TEXTURE2*/0x84C2,/*laya.webgl.WebGLContext.TEXTURE3*/0x84C3,/*laya.webgl.WebGLContext.TEXTURE4*/0x84C4,/*laya.webgl.WebGLContext.TEXTURE5*/0x84C5,/*laya.webgl.WebGLContext.TEXTURE6*/0x84C6,/*laya.webgl.WebGLContext.TEXTURE7*/0x84C7];
	Shader3D._count=0;
	__static(Shader3D,
	['shaderParamsMap',function(){return this.shaderParamsMap={"float":/*laya.webgl.WebGLContext.FLOAT*/0x1406,"int":/*laya.webgl.WebGLContext.INT*/0x1404,"bool":/*laya.webgl.WebGLContext.BOOL*/0x8B56,"vec2":/*laya.webgl.WebGLContext.FLOAT_VEC2*/0x8B50,"vec3":/*laya.webgl.WebGLContext.FLOAT_VEC3*/0x8B51,"vec4":/*laya.webgl.WebGLContext.FLOAT_VEC4*/0x8B52,"ivec2":/*laya.webgl.WebGLContext.INT_VEC2*/0x8B53,"ivec3":/*laya.webgl.WebGLContext.INT_VEC3*/0x8B54,"ivec4":/*laya.webgl.WebGLContext.INT_VEC4*/0x8B55,"bvec2":/*laya.webgl.WebGLContext.BOOL_VEC2*/0x8B57,"bvec3":/*laya.webgl.WebGLContext.BOOL_VEC3*/0x8B58,"bvec4":/*laya.webgl.WebGLContext.BOOL_VEC4*/0x8B59,"mat2":/*laya.webgl.WebGLContext.FLOAT_MAT2*/0x8B5A,"mat3":/*laya.webgl.WebGLContext.FLOAT_MAT3*/0x8B5B,"mat4":/*laya.webgl.WebGLContext.FLOAT_MAT4*/0x8B5C,"sampler2D":/*laya.webgl.WebGLContext.SAMPLER_2D*/0x8B5E,"samplerCube":/*laya.webgl.WebGLContext.SAMPLER_CUBE*/0x8B60};},'nameKey',function(){return this.nameKey=new StringKey();}
	]);
	return Shader3D;
})(BaseShader)


/**
*...
*@author ...
*/
//class laya.d3.core.material.StandardMaterial extends laya.d3.core.material.BaseMaterial
var StandardMaterial=(function(_super){
	function StandardMaterial(){
		/**@private */
		this._transformUV=null;
		StandardMaterial.__super.call(this);
		this.setShaderName("SIMPLE");
		this._setColor(9,new Vector3(0.6,0.6,0.6));
		this._setColor(10,new Vector3(1.0,1.0,1.0));
		this._setColor(11,new Vector4(1.0,1.0,1.0,8.0));
		this._setColor(12,new Vector3(1.0,1.0,1.0));
		this._setColor(7,new Vector4(1.0,1.0,1.0,1.0));
		this._setNumber(0,0.5);
		this._setColor(15,new Vector4(1.0,1.0,0.0,0.0));
		this.renderMode=1;
	}

	__class(StandardMaterial,'laya.d3.core.material.StandardMaterial',_super);
	var __proto=StandardMaterial.prototype;
	/**
	*禁用灯光。
	*/
	__proto.disableLight=function(){
		this._addDisablePublicShaderDefine(ShaderCompile3D.SHADERDEFINE_POINTLIGHT | ShaderCompile3D.SHADERDEFINE_SPOTLIGHT | ShaderCompile3D.SHADERDEFINE_DIRECTIONLIGHT);
	}

	/**
	*禁用雾化。
	*/
	__proto.disableFog=function(){
		this._addDisablePublicShaderDefine(ShaderCompile3D.SHADERDEFINE_FOG);
	}

	/**
	*@inheritDoc
	*/
	__proto.onAsynLoaded=function(url,data,params){
		var jsonData=data[0];
		if (jsonData.version){
			_super.prototype.onAsynLoaded.call(this,url,data,params);
			}else {
			var textureMap=data[1];
			var props=jsonData.props;
			for (var prop in props)
			this[prop]=props[prop];
			StandardMaterial._parseStandardMaterial(textureMap,this,jsonData);
			this._endLoaded();
		}
	}

	/**
	*@inheritDoc
	*/
	__proto.cloneTo=function(destObject){
		_super.prototype.cloneTo.call(this,destObject);
		var dest=destObject;
		(this._transformUV)&& (dest._transformUV=this._transformUV.clone());
	}

	/**
	*设置环境光颜色。
	*@param value 环境光颜色。
	*/
	__getset(0,__proto,'ambientColor',function(){
		return this._getColor(9);
		},function(value){
		this._setColor(9,value);
	});

	/**
	*设置环境贴图。
	*@param value 环境贴图。
	*/
	/**
	*获取环境贴图。
	*@return 环境贴图。
	*/
	__getset(0,__proto,'ambientTexture',function(){
		return this._getTexture(5);
		},function(value){
		if (value){
			this._addShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_AMBIENTMAP);
			}else {
			this._removeShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_AMBIENTMAP);
		}
		this._setTexture(5,value);
	});

	/**
	*设置渲染模式。
	*@return 渲染模式。
	*/
	__getset(0,__proto,'renderMode',null,function(value){
		switch (value){
			case 1:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.OPAQUE*/1;
				this.depthWrite=true;
				this.cull=2;
				this.blend=0;
				this.alphaTest=false;
				this._removeShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 2:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.OPAQUE*/1;
				this.depthWrite=true;
				this.cull=0;
				this.blend=0;
				this.alphaTest=false;
				this._removeShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 3:
				this.depthWrite=true;
				this.cull=2;
				this.blend=0;
				this.renderQueue=/*laya.d3.core.render.RenderQueue.OPAQUE*/1;
				this.alphaTest=true;
				this._removeShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 4:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.OPAQUE*/1;
				this.depthWrite=true;
				this.cull=0;
				this.blend=0;
				this.alphaTest=true;
				this._removeShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 13:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthWrite=true;
				this.cull=2;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=0x0303;
				this.alphaTest=false;
				this._removeShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 14:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthWrite=true;
				this.cull=0;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=0x0303;
				this.alphaTest=false;
				this._removeShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 15:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthWrite=true;
				this.cull=2;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=1;
				this.alphaTest=false;
				this._addShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 16:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthWrite=true;
				this.cull=0;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=1;
				this.alphaTest=false;
				this._addShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 5:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthWrite=false;
				this.cull=2;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=0x0303;
				this.alphaTest=false;
				this._removeShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 6:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthWrite=false;
				this.cull=0;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=0x0303;
				this.alphaTest=false;
				this._removeShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 7:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthWrite=false;
				this.cull=2;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=1;
				this.alphaTest=false;
				this._addShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 8:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthWrite=false;
				this.cull=0;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=1;
				this.alphaTest=false;
				this._addShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 9:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthTest=0x0201;
				this.cull=2;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=0x0303;
				this.alphaTest=false;
				this._removeShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 10:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthTest=0x0201;
				this.cull=0;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=0x0303;
				this.alphaTest=false;
				this._removeShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 11:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthTest=0x0201;
				this.cull=2;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=1;
				this.alphaTest=false;
				this._addShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			case 12:
				this.renderQueue=/*laya.d3.core.render.RenderQueue.TRANSPARENT*/2;
				this.depthTest=0x0201;
				this.cull=0;
				this.blend=1;
				this.srcBlend=0x0302;
				this.dstBlend=1;
				this.alphaTest=false;
				this._addShaderDefine(StandardMaterial.SHADERDEFINE_ADDTIVEFOG);
				break ;
			default :
				throw new Error("Material:renderMode value error.");
			}
		this._conchMaterial && this._conchMaterial.setRenderMode(value);
	});

	/**
	*设置反射颜色。
	*@param value 反射颜色。
	*/
	__getset(0,__proto,'reflectColor',function(){
		return this._getColor(12);
		},function(value){
		this._setColor(12,value);
	});

	/**
	*获取纹理平铺和偏移。
	*@param value 纹理平铺和偏移。
	*/
	/**
	*获取纹理平铺和偏移。
	*@return 纹理平铺和偏移。
	*/
	__getset(0,__proto,'tilingOffset',function(){
		return this._getColor(15);
		},function(value){
		if (value){
			var valueE=value.elements;
			if (valueE[0] !=1 || valueE[1] !=1 || valueE[2] !=0 || valueE[3] !=0)
				this._addShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_TILINGOFFSET);
			else
			this._removeShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_TILINGOFFSET);
			}else {
			this._removeShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_TILINGOFFSET);
		}
		this._setColor(15,value);
	});

	/**
	*设置反射率。
	*@param value 反射率。
	*/
	__getset(0,__proto,'albedo',function(){
		return this._getColor(7);
		},function(value){
		this._setColor(7,value);
	});

	/**
	*设置漫反射光颜色。
	*@param value 漫反射光颜色。
	*/
	__getset(0,__proto,'diffuseColor',function(){
		return this._getColor(10);
		},function(value){
		this._setColor(10,value);
	});

	/**
	*设置反射率。
	*@param value 反射率。
	*/
	__getset(0,__proto,'albedoColor',function(){
		return this._getColor(7);
		},function(value){
		this._setColor(7,value);
	});

	/**
	*设置高光颜色。
	*@param value 高光颜色。
	*/
	__getset(0,__proto,'specularColor',function(){
		return this._getColor(11);
		},function(value){
		this._setColor(11,value);
	});

	/**
	*设置漫反射贴图。
	*@param value 漫反射贴图。
	*/
	/**
	*获取漫反射贴图。
	*@return 漫反射贴图。
	*/
	__getset(0,__proto,'diffuseTexture',function(){
		return this._getTexture(1);
		},function(value){
		if (value){
			this._addShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_DIFFUSEMAP);
			}else {
			this._removeShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_DIFFUSEMAP);
		}
		this._setTexture(1,value);
	});

	/**
	*设置法线贴图。
	*@param value 法线贴图。
	*/
	/**
	*获取法线贴图。
	*@return 法线贴图。
	*/
	__getset(0,__proto,'normalTexture',function(){
		return this._getTexture(2);
		},function(value){
		if (value){
			this._addShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_NORMALMAP);
			}else {
			this._removeShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_NORMALMAP);
		}
		this._setTexture(2,value);
	});

	/**
	*设置高光贴图。
	*@param value 高光贴图。
	*/
	/**
	*获取高光贴图。
	*@return 高光贴图。
	*/
	__getset(0,__proto,'specularTexture',function(){
		return this._getTexture(3);
		},function(value){
		if (value){
			this._addShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_SPECULARMAP);
			}else {
			this._removeShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_SPECULARMAP);
		}
		this._setTexture(3,value);
	});

	/**
	*设置放射贴图。
	*@param value 放射贴图。
	*/
	/**
	*获取放射贴图。
	*@return 放射贴图。
	*/
	__getset(0,__proto,'emissiveTexture',function(){
		return this._getTexture(4);
		},function(value){
		if (value){
			this._addShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_EMISSIVEMAP);
			}else {
			this._removeShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_EMISSIVEMAP);
		}
		this._setTexture(4,value);
	});

	/**
	*设置反射贴图。
	*@param value 反射贴图。
	*/
	/**
	*获取反射贴图。
	*@return 反射贴图。
	*/
	__getset(0,__proto,'reflectTexture',function(){
		return this._getTexture(6);
		},function(value){
		if (value){
			this._addShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_REFLECTMAP);
			}else {
			this._removeShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_REFLECTMAP);
		}
		this._setTexture(6,value);
	});

	/**
	*设置UV变换。
	*@param value UV变换。
	*/
	/**
	*获取UV变换。
	*@return UV变换。
	*/
	__getset(0,__proto,'transformUV',function(){
		return this._transformUV;
		},function(value){
		this._transformUV=value;
		this._setMatrix4x4(13,value.matrix);
		if (value)
			this._addShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_UVTRANSFORM);
		else
		this._removeShaderDefine(laya.d3.core.material.StandardMaterial.SHADERDEFINE_UVTRANSFORM);
		if (this._conchMaterial){
			this._conchMaterial.setShaderValue(13,value.matrix.elements,0);
		}
	});

	StandardMaterial.__init__=function(){
		StandardMaterial.SHADERDEFINE_DIFFUSEMAP=StandardMaterial.shaderDefines.registerDefine("DIFFUSEMAP");
		StandardMaterial.SHADERDEFINE_NORMALMAP=StandardMaterial.shaderDefines.registerDefine("NORMALMAP");
		StandardMaterial.SHADERDEFINE_SPECULARMAP=StandardMaterial.shaderDefines.registerDefine("SPECULARMAP");
		StandardMaterial.SHADERDEFINE_EMISSIVEMAP=StandardMaterial.shaderDefines.registerDefine("EMISSIVEMAP");
		StandardMaterial.SHADERDEFINE_AMBIENTMAP=StandardMaterial.shaderDefines.registerDefine("AMBIENTMAP");
		StandardMaterial.SHADERDEFINE_REFLECTMAP=StandardMaterial.shaderDefines.registerDefine("REFLECTMAP");
		StandardMaterial.SHADERDEFINE_UVTRANSFORM=StandardMaterial.shaderDefines.registerDefine("UVTRANSFORM");
		StandardMaterial.SHADERDEFINE_TILINGOFFSET=StandardMaterial.shaderDefines.registerDefine("TILINGOFFSET");
		StandardMaterial.SHADERDEFINE_ADDTIVEFOG=StandardMaterial.shaderDefines.registerDefine("ADDTIVEFOG");
	}

	StandardMaterial.load=function(url){
		return Laya.loader.create(url,null,null,StandardMaterial);
	}

	StandardMaterial._parseStandardMaterial=function(textureMap,material,json){
		var customProps=json.customProps;
		var ambientColorValue=customProps.ambientColor;
		material.ambientColor=new Vector3(ambientColorValue[0],ambientColorValue[1],ambientColorValue[2]);
		var diffuseColorValue=customProps.diffuseColor;
		material.diffuseColor=new Vector3(diffuseColorValue[0],diffuseColorValue[1],diffuseColorValue[2]);
		var specularColorValue=customProps.specularColor;
		material.specularColor=new Vector4(specularColorValue[0],specularColorValue[1],specularColorValue[2],specularColorValue[3]);
		var reflectColorValue=customProps.reflectColor;
		material.reflectColor=new Vector3(reflectColorValue[0],reflectColorValue[1],reflectColorValue[2]);
		var albedoColorValue=customProps.albedoColor;
		(albedoColorValue)&& (material.albedo=new Vector4(albedoColorValue[0],albedoColorValue[1],albedoColorValue[2],albedoColorValue[3]));
		var diffuseTexture=customProps.diffuseTexture.texture2D;
		(diffuseTexture)&& (material.diffuseTexture=Loader.getRes(textureMap[diffuseTexture]));
		var normalTexture=customProps.normalTexture.texture2D;
		(normalTexture)&& (material.normalTexture=Loader.getRes(textureMap[normalTexture]));
		var specularTexture=customProps.specularTexture.texture2D;
		(specularTexture)&& (material.specularTexture=Loader.getRes(textureMap[specularTexture]));
		var emissiveTexture=customProps.emissiveTexture.texture2D;
		(emissiveTexture)&& (material.emissiveTexture=Loader.getRes(textureMap[emissiveTexture]));
		var ambientTexture=customProps.ambientTexture.texture2D;
		(ambientTexture)&& (material.ambientTexture=Loader.getRes(textureMap[ambientTexture]));
		var reflectTexture=customProps.reflectTexture.texture2D;
		(reflectTexture)&& (material.reflectTexture=Loader.getRes(textureMap[reflectTexture]));
	}

	StandardMaterial.RENDERMODE_OPAQUE=1;
	StandardMaterial.RENDERMODE_OPAQUEDOUBLEFACE=2;
	StandardMaterial.RENDERMODE_CUTOUT=3;
	StandardMaterial.RENDERMODE_CUTOUTDOUBLEFACE=4;
	StandardMaterial.RENDERMODE_TRANSPARENT=13;
	StandardMaterial.RENDERMODE_TRANSPARENTDOUBLEFACE=14;
	StandardMaterial.RENDERMODE_ADDTIVE=15;
	StandardMaterial.RENDERMODE_ADDTIVEDOUBLEFACE=16;
	StandardMaterial.RENDERMODE_DEPTHREAD_TRANSPARENT=5;
	StandardMaterial.RENDERMODE_DEPTHREAD_TRANSPARENTDOUBLEFACE=6;
	StandardMaterial.RENDERMODE_DEPTHREAD_ADDTIVE=7;
	StandardMaterial.RENDERMODE_DEPTHREAD_ADDTIVEDOUBLEFACE=8;
	StandardMaterial.RENDERMODE_NONDEPTH_TRANSPARENT=9;
	StandardMaterial.RENDERMODE_NONDEPTH_TRANSPARENTDOUBLEFACE=10;
	StandardMaterial.RENDERMODE_NONDEPTH_ADDTIVE=11;
	StandardMaterial.RENDERMODE_NONDEPTH_ADDTIVEDOUBLEFACE=12;
	StandardMaterial.SHADERDEFINE_DIFFUSEMAP=0;
	StandardMaterial.SHADERDEFINE_NORMALMAP=0;
	StandardMaterial.SHADERDEFINE_SPECULARMAP=0;
	StandardMaterial.SHADERDEFINE_EMISSIVEMAP=0;
	StandardMaterial.SHADERDEFINE_AMBIENTMAP=0;
	StandardMaterial.SHADERDEFINE_REFLECTMAP=0;
	StandardMaterial.SHADERDEFINE_UVTRANSFORM=0;
	StandardMaterial.SHADERDEFINE_TILINGOFFSET=0;
	StandardMaterial.SHADERDEFINE_ADDTIVEFOG=0;
	StandardMaterial.DIFFUSETEXTURE=1;
	StandardMaterial.NORMALTEXTURE=2;
	StandardMaterial.SPECULARTEXTURE=3;
	StandardMaterial.EMISSIVETEXTURE=4;
	StandardMaterial.AMBIENTTEXTURE=5;
	StandardMaterial.REFLECTTEXTURE=6;
	StandardMaterial.ALBEDO=7;
	StandardMaterial.UVANIAGE=8;
	StandardMaterial.MATERIALAMBIENT=9;
	StandardMaterial.MATERIALDIFFUSE=10;
	StandardMaterial.MATERIALSPECULAR=11;
	StandardMaterial.MATERIALREFLECT=12;
	StandardMaterial.UVMATRIX=13;
	StandardMaterial.UVAGE=14;
	StandardMaterial.TILINGOFFSET=15;
	__static(StandardMaterial,
	['defaultMaterial',function(){return this.defaultMaterial=new StandardMaterial();},'shaderDefines',function(){return this.shaderDefines=new ShaderDefines$1(BaseMaterial.shaderDefines);}
	]);
	return StandardMaterial;
})(BaseMaterial)



/**
*<code>IndexBuffer3D</code> 类用于创建索引缓冲。
*/
//class laya.d3.graphics.IndexBuffer3D extends laya.webgl.utils.Buffer
var IndexBuffer3D=(function(_super){
	function IndexBuffer3D(indexType,indexCount,bufferUsage,canRead){
		/**@private */
		this._indexType=null;
		/**@private */
		this._indexTypeByteCount=0;
		/**@private */
		this._indexCount=0;
		/**@private */
		this._canRead=false;
		(bufferUsage===void 0)&& (bufferUsage=/*laya.webgl.WebGLContext.STATIC_DRAW*/0x88E4);
		(canRead===void 0)&& (canRead=false);
		IndexBuffer3D.__super.call(this);
		this._indexType=indexType;
		this._indexCount=indexCount;
		this._bufferUsage=bufferUsage;
		this._bufferType=/*laya.webgl.WebGLContext.ELEMENT_ARRAY_BUFFER*/0x8893;
		this._canRead=canRead;
		var byteLength=0;
		if (indexType==/*CLASS CONST:laya.d3.graphics.IndexBuffer3D.INDEXTYPE_USHORT*/"ushort")
			this._indexTypeByteCount=2;
		else if (indexType==/*CLASS CONST:laya.d3.graphics.IndexBuffer3D.INDEXTYPE_UBYTE*/"ubyte")
		this._indexTypeByteCount=1;
		else
		throw new Error("unidentification index type.");
		byteLength=this._indexTypeByteCount *indexCount;
		this._byteLength=byteLength;
		this._bind();
		Buffer._gl.bufferData(this._bufferType,byteLength,this._bufferUsage);
		if (canRead){
			if (indexType==/*CLASS CONST:laya.d3.graphics.IndexBuffer3D.INDEXTYPE_USHORT*/"ushort")
				this._buffer=new Uint16Array(indexCount);
			else if (indexType==/*CLASS CONST:laya.d3.graphics.IndexBuffer3D.INDEXTYPE_UBYTE*/"ubyte")
			this._buffer=new Uint8Array(indexCount);
			this.memorySize=byteLength *2;
			}else {
			this.memorySize=byteLength;
		}
	}

	__class(IndexBuffer3D,'laya.d3.graphics.IndexBuffer3D',_super);
	var __proto=IndexBuffer3D.prototype;
	/**
	*设置数据。
	*@param data 索引数据。
	*@param bufferOffset 索引缓冲中的偏移。
	*@param dataStartIndex 索引数据的偏移。
	*@param dataCount 索引数据的数量。
	*/
	__proto.setData=function(data,bufferOffset,dataStartIndex,dataCount){
		(bufferOffset===void 0)&& (bufferOffset=0);
		(dataStartIndex===void 0)&& (dataStartIndex=0);
		(dataCount===void 0)&& (dataCount=4294967295);
		var byteCount=0;
		if (this._indexType==/*CLASS CONST:laya.d3.graphics.IndexBuffer3D.INDEXTYPE_USHORT*/"ushort"){
			byteCount=2;
			if (dataStartIndex!==0 || dataCount!==4294967295)
				data=new Uint16Array(data.buffer,dataStartIndex *byteCount,dataCount);
			}else if (this._indexType==/*CLASS CONST:laya.d3.graphics.IndexBuffer3D.INDEXTYPE_UBYTE*/"ubyte"){
			byteCount=1;
			if (dataStartIndex!==0 || dataCount!==4294967295)
				data=new Uint8Array(data.buffer,dataStartIndex *byteCount,dataCount);
		}
		this._bind();
		Buffer._gl.bufferSubData(this._bufferType,bufferOffset *byteCount,data);
		if (this._canRead){
			if (bufferOffset!==0 || dataStartIndex!==0 || dataCount!==4294967295){
				var maxLength=this._buffer.length-bufferOffset;
				if (dataCount > maxLength)
					dataCount=maxLength;
				for (var i=0;i < dataCount;i++)
				this._buffer[bufferOffset+i]=data[i];
				}else {
				this._buffer=data;
			}
		}
	}

	/**
	*获取索引数据。
	*@return 索引数据。
	*/
	__proto.getData=function(){
		if (this._canRead)
			return this._buffer;
		else
		throw new Error("Can't read data from VertexBuffer with only write flag!");
	}

	/**
	*@inheritDoc
	*/
	__proto.disposeResource=function(){
		_super.prototype.disposeResource.call(this);
		this._buffer=null;
		this.memorySize=0;
	}

	/**
	*获取索引类型。
	*@return 索引类型。
	*/
	__getset(0,__proto,'indexType',function(){
		return this._indexType;
	});

	/**
	*获取索引类型字节数量。
	*@return 索引类型字节数量。
	*/
	__getset(0,__proto,'indexTypeByteCount',function(){
		return this._indexTypeByteCount;
	});

	/**
	*获取索引个数。
	*@return 索引个数。
	*/
	__getset(0,__proto,'indexCount',function(){
		return this._indexCount;
	});

	/**
	*获取是否可读。
	*@return 是否可读。
	*/
	__getset(0,__proto,'canRead',function(){
		return this._canRead;
	});

	IndexBuffer3D.INDEXTYPE_UBYTE="ubyte";
	IndexBuffer3D.INDEXTYPE_USHORT="ushort";
	IndexBuffer3D.create=function(indexType,indexCount,bufferUsage,canRead){
		(bufferUsage===void 0)&& (bufferUsage=/*laya.webgl.WebGLContext.STATIC_DRAW*/0x88E4);
		(canRead===void 0)&& (canRead=false);
		return new IndexBuffer3D(indexType,indexCount,bufferUsage,canRead);
	}

	return IndexBuffer3D;
})(Buffer)

/**
*<code>VertexBuffer3D</code> 类用于创建顶点缓冲。
*/
//class laya.d3.graphics.VertexBuffer3D extends laya.webgl.utils.Buffer
var VertexBuffer3D=(function(_super){
	function VertexBuffer3D(vertexDeclaration,vertexCount,bufferUsage,canRead){
		/**@private */
		this._vertexDeclaration=null;
		/**@private */
		this._vertexCount=0;
		/**@private */
		this._canRead=false;
		(canRead===void 0)&& (canRead=false);
		VertexBuffer3D.__super.call(this);
		this._vertexDeclaration=vertexDeclaration;
		this._vertexCount=vertexCount;
		this._bufferUsage=bufferUsage;
		this._bufferType=/*laya.webgl.WebGLContext.ARRAY_BUFFER*/0x8892;
		this._canRead=canRead;
		this.memorySize=this._byteLength=this._vertexDeclaration.vertexStride *vertexCount;
		this._bind();
		Buffer._gl.bufferData(this._bufferType,this._byteLength,this._bufferUsage);
		canRead && (this._buffer=new Float32Array(this._byteLength / 4));
	}

	__class(VertexBuffer3D,'laya.d3.graphics.VertexBuffer3D',_super);
	var __proto=VertexBuffer3D.prototype;
	/**
	*和索引缓冲一起绑定。
	*@param ib 索引缓冲。
	*/
	__proto.bindWithIndexBuffer=function(ib){
		(ib)&& (ib._bind());
		this._bind();
	}

	/**
	*设置数据。
	*@param data 顶点数据。
	*@param bufferOffset 顶点缓冲中的偏移。
	*@param dataStartIndex 顶点数据的偏移。
	*@param dataCount 顶点数据的数量。
	*/
	__proto.setData=function(data,bufferOffset,dataStartIndex,dataCount){
		(bufferOffset===void 0)&& (bufferOffset=0);
		(dataStartIndex===void 0)&& (dataStartIndex=0);
		(dataCount===void 0)&& (dataCount=4294967295);
		if (dataStartIndex!==0 || dataCount!==4294967295)
			data=new Float32Array(data.buffer,dataStartIndex *4,dataCount);
		this._bind();
		Buffer._gl.bufferSubData(this._bufferType,bufferOffset *4,data);
		if (this._canRead){
			if (bufferOffset!==0 || dataStartIndex!==0 || dataCount!==4294967295){
				var maxLength=this._buffer.length-bufferOffset;
				if (dataCount > maxLength)
					dataCount=maxLength;
				for (var i=0;i < dataCount;i++)
				this._buffer[bufferOffset+i]=data[i];
				}else {
				this._buffer=data;
			}
		}
	}

	/**
	*获取顶点数据。
	*@return 顶点数据。
	*/
	__proto.getData=function(){
		if (this._canRead)
			return this._buffer;
		else
		throw new Error("Can't read data from VertexBuffer with only write flag!");
	}

	/**销毁顶点缓冲。*/
	__proto.disposeResource=function(){
		var gl=WebGL.mainContext;
		var elements=this._vertexDeclaration.getVertexElements();
		var enableAtributes=Buffer._enableAtributes;
		for (var i=0,n=elements.length;i < n;i++){
			if (enableAtributes[i]===this._glBuffer){
				gl.disableVertexAttribArray(i);
				enableAtributes[i]=null;
			}
		}
		_super.prototype.disposeResource.call(this);
		this._buffer=null;
		this._vertexDeclaration=null;
		this.memorySize=0;
	}

	/**
	*获取顶点结构声明。
	*@return 顶点结构声明。
	*/
	__getset(0,__proto,'vertexDeclaration',function(){
		return this._vertexDeclaration;
	});

	/**
	*获取顶点个数。
	*@return 顶点个数。
	*/
	__getset(0,__proto,'vertexCount',function(){
		return this._vertexCount;
	});

	/**
	*获取是否可读。
	*@return 是否可读。
	*/
	__getset(0,__proto,'canRead',function(){
		return this._canRead;
	});

	VertexBuffer3D.create=function(vertexDeclaration,vertexCount,bufferUsage,canRead){
		(bufferUsage===void 0)&& (bufferUsage=/*laya.webgl.WebGLContext.STATIC_DRAW*/0x88E4);
		(canRead===void 0)&& (canRead=false);
		return new VertexBuffer3D(vertexDeclaration,vertexCount,bufferUsage,canRead);
	}

	return VertexBuffer3D;
})(Buffer)


//class laya.d3.resource.DataTexture2D extends laya.d3.resource.BaseTexture
var DataTexture2D=(function(_super){
	function DataTexture2D(){
		this.simLodInfo=null;
		/**@private 文件路径全名。*/
		this._src=null;
		/**@private Imagedata */
		this._buffer=null;
		this._mipmaps=null;
		/**@private 异步加载锁*/
		this._recreateLock=false;
		/**@private 异步加载完成后是否需要释放（有可能在恢复过程中,再次被释放，用此变量做标记）*/
		this._needReleaseAgain=false;
		DataTexture2D.__super.call(this);
		this._type=/*laya.webgl.WebGLContext.TEXTURE_2D*/0x0DE1;
	}

	__class(DataTexture2D,'laya.d3.resource.DataTexture2D',_super);
	var __proto=DataTexture2D.prototype;
	__proto.genDebugMipmaps=function(){
		var ret=[];
		ret.push(new Uint8Array((new Uint32Array(512 *256)).fill(0xff0000ff).buffer));
		ret.push(new Uint8Array((new Uint32Array(256 *128)).fill(0xff0080ff).buffer));
		ret.push(new Uint8Array((new Uint32Array(128 *64)).fill(0xff00ffff).buffer));
		ret.push(new Uint8Array((new Uint32Array(64 *32)).fill(0xff00ff00).buffer));
		ret.push(new Uint8Array((new Uint32Array(32 *16)).fill(0xff804000).buffer));
		ret.push(new Uint8Array((new Uint32Array(16 *8)).fill(0xffff0000).buffer));
		ret.push(new Uint8Array((new Uint32Array(8 *4)).fill(0xffff0080).buffer));
		ret.push(new Uint8Array((new Uint32Array(4 *2)).fill(0x0).buffer));
		ret.push(new Uint8Array((new Uint32Array(2 *1)).fill(0xff808080).buffer));
		ret.push(new Uint8Array((new Uint32Array(1 *1)).fill(0xffffffff).buffer));
		return ret;
	}

	/**
	*@private
	*/
	__proto._onTextureLoaded=function(buff){}
	/**
	*@private
	*/
	__proto._createWebGlTexture=function(){
		if (!this._buffer && !this._mipmaps)
			throw "create GLTextur err:no data";
		var gl=WebGL.mainContext;
		gl.getExtension("EXT_shader_texture_lod");
		var glTexture=this._source=gl.createTexture();
		var w=this._width;
		var h=this._height;
		var preTarget=WebGLContext.curBindTexTarget;
		var preTexture=WebGLContext.curBindTexValue;
		WebGLContext.bindTexture(gl,this._type,glTexture);
		if (this._mipmaps){
			if (laya.d3.resource.DataTexture2D.lodasatlas){
				var infoi=0;
				gl.texImage2D(this._type,0,/*laya.webgl.WebGLContext.RGBA*/0x1908,this._width,this._height,0,/*laya.webgl.WebGLContext.RGBA*/0x1908,/*laya.webgl.WebGLContext.UNSIGNED_BYTE*/0x1401,null);
				for (var i=0;i < this._mipmaps.length;i++){
					if (this._mipmaps[i].byteLength !=cw *ch *4){
						throw "mipmap size error  level:"+i;
					}
					gl.texSubImage2D(this._type,0,DataTexture2D.simLodRect[infoi++],DataTexture2D.simLodRect[infoi++],DataTexture2D.simLodRect[infoi++],DataTexture2D.simLodRect[infoi++],/*laya.webgl.WebGLContext.RGBA*/0x1908,/*laya.webgl.WebGLContext.UNSIGNED_BYTE*/0x1401,new Uint8Array(this._mipmaps[i]));
				}
				this.minFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601;
				this.magFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601;
				}else {
				var cw=this._width;
				var ch=this._height;
				infoi=0;
				gl.texImage2D(this._type,0,/*laya.webgl.WebGLContext.RGBA*/0x1908,this._width,this._height,0,/*laya.webgl.WebGLContext.RGBA*/0x1908,/*laya.webgl.WebGLContext.UNSIGNED_BYTE*/0x1401,null);
				for (i=0;i < this._mipmaps.length;i++){
					if (this._mipmaps[i].byteLength !=cw *ch *4){
						throw "mipmap size error  level:"+i;
					}
					gl.texImage2D(this._type,i,/*laya.webgl.WebGLContext.RGBA*/0x1908,cw,ch,0,/*laya.webgl.WebGLContext.RGBA*/0x1908,/*laya.webgl.WebGLContext.UNSIGNED_BYTE*/0x1401,new Uint8Array(this._mipmaps[i]));
					cw /=2;
					ch /=2;
					if (cw < 1)cw=1;
					if (ch < 1)ch=1;
					this.minFifter=/*laya.webgl.WebGLContext.LINEAR_MIPMAP_LINEAR*/0x2703;
					this.magFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601;
				}
			}
			this.mipmap=false;
			}else {
			gl.texImage2D(this._type,0,/*laya.webgl.WebGLContext.RGBA*/0x1908,w,h,0,/*laya.webgl.WebGLContext.RGBA*/0x1908,/*laya.webgl.WebGLContext.UNSIGNED_BYTE*/0x1401,new Uint8Array(this._buffer));
		};
		var minFifter=this._minFifter;
		var magFifter=this._magFifter;
		var repeat=this._repeat ? /*laya.webgl.WebGLContext.REPEAT*/0x2901 :/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F;
		var isPot=Arith.isPOT(w,h);
		if (isPot){
			if (this._mipmap || this._mipmaps)
				(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR_MIPMAP_LINEAR*/0x2703);
			else
			(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			(magFifter!==-1)|| (magFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MIN_FILTER*/0x2801,minFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MAG_FILTER*/0x2800,magFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_S*/0x2802,repeat);
			if (this._mipmaps)
				gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_T*/0x2803,/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F);
			else
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_T*/0x2803,repeat);
			this._mipmap && gl.generateMipmap(this._type);
			}else {
			throw "data texture must be POT";
		}
		(preTarget && preTexture)&& (WebGLContext.bindTexture(gl,preTarget,preTexture));
		if (this.src && this.src.length > 0)
			this._buffer=null;
		if (isPot)
			this.memorySize=w *h *4 *(1+1 / 3);
		else
		this.memorySize=w *h *4;
		this._recreateLock=false;
	}

	/**
	*重新创建资源，如果异步创建中被强制释放再创建，则需等待释放完成后再重新加载创建。
	*/
	__proto.recreateResource=function(){
		if (!this._buffer && (this._src==null || this._src===""))
			return;
		this._needReleaseAgain=false;
		if (!this._buffer && !this._mipmaps){
			this._recreateLock=true;
			var _this=this;
			}else {
			if (this._recreateLock){
				return;
			}
			this._createWebGlTexture();
			this.completeCreate();
		}
	}

	/**
	*@private
	*/
	__proto.onAsynLoaded=function(url,data,params){
		var imgdata;
		if (params){
			imgdata=params[0].call(this,data);
		}
		if (imgdata){
			this._width=imgdata.width;
			this._height=imgdata.height;
			this._buffer=imgdata.data;
		}
		this._src=url;
		this._size=new Size(this._width,this._height);
		if (this._conchTexture){
			alert('怎么给runtime传递datatexture数据');
		}else
		this.activeResource();
		this._endLoaded();
	}

	/**
	*返回图片像素。
	*@return 图片像素。
	*/
	__proto.getPixels=function(){
		return new Uint8Array(this._buffer);
	}

	/**
	*销毁资源。
	*/
	__proto.disposeResource=function(){
		if (this._recreateLock){
			this._needReleaseAgain=true;
		}
		if (this._source){
			WebGL.mainContext.deleteTexture(this._source);
			this._source=null;
			this._buffer=null;
			this.memorySize=0;
		}
	}

	/**
	*获取文件路径全名。
	*/
	__getset(0,__proto,'src',function(){
		return this._src;
	});

	DataTexture2D.create=function(data,w,h,magfilter,minfilter,mipmap){
		(magfilter===void 0)&& (magfilter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
		(minfilter===void 0)&& (minfilter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
		(mipmap===void 0)&& (mipmap=true);
		if (!data || data.byteLength < (w *h *4))
			throw 'DataTexture2D create error';
		var ret=new DataTexture2D();
		ret._buffer=data;
		ret._width=w;
		ret._height=h;
		ret._mipmap=mipmap;
		ret._magFifter=magfilter;
		ret._minFifter=minfilter;
		ret._size=new Size(ret._width,ret._height);
		if (ret._conchTexture){
			alert('怎么给runtime传递datatexture数据');
		}else
		ret.activeResource();
		return ret;
	}

	DataTexture2D.load=function(url,w,h,magfilter,minfilter){
		(w===void 0)&& (w=0);
		(h===void 0)&& (h=0);
		(magfilter===void 0)&& (magfilter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
		(minfilter===void 0)&& (minfilter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
		var extension=Utils.getFileExtension(url);
		if (extension==='mipmaps'){
			var ret=Laya.loader.create(url,null,null,DataTexture2D,[function(data){
				this._mipmaps=[];
				var szinfo=new Uint32Array(data);
				this._width=szinfo[0];
				var validw=512;
				if (laya.d3.resource.DataTexture2D.lodasatlas){
					this._width *=2;
					validw=1024;
				}
				if (this._width !=validw){
					console.error("现在只支持512x256的环境贴图。当前的是"+szinfo[0]);
					throw "现在只支持512x256的环境贴图。当前的是"+szinfo[0];
				}
				this._height=szinfo[1];
				var curw=laya.d3.resource.DataTexture2D.lodasatlas ? this._width / 2 :this._width;
				var curh=this._height;
				var cursz=8;
				while (true){
					var curbufsz=curw *curh *4;
					if (cursz+curbufsz > data.byteLength){
						throw "load mipmaps data size error ";
					};
					var tbuf=new Uint8Array(data,cursz,curbufsz);
					this._mipmaps.push(tbuf);
					cursz+=curbufsz;
					if (curw==1 && curh==1){
						break ;
					}
					curw /=2;
					curh /=2;
					if (curw < 1)curw=1;
					if (curh < 1)curh=1;
				}
				return null;
			}]);
			if (laya.d3.resource.DataTexture2D.lodasatlas){
				ret.simLodInfo=new Float32Array(40);
				for (var i=0;i < ret.simLodInfo.length;){
					ret.simLodInfo[i]=(DataTexture2D.simLodRect[i]+0.5)/ 1024;
					i++;
					ret.simLodInfo[i]=(DataTexture2D.simLodRect[i]+0.5)/ 256;
					i++;
					ret.simLodInfo[i]=Math.max(DataTexture2D.simLodRect[i]-1,0.1)/ 1024;
					i++;
					ret.simLodInfo[i]=Math.max(DataTexture2D.simLodRect[i]-1.5,0.1)/ 256;
					i++;
				}
			}
			return ret;
			}else if (typeof(w)=='number'){
			return Laya.loader.create(url,null,null,DataTexture2D,[function(data){
				this._width=w;
				this._height=h;
				this._buffer=data;
				return null;
			}]);
			}else if (typeof(w)=='function'){
			return Laya.loader.create(url,null,null,DataTexture2D,[w]);
			}else {
			throw new Error("unknown params.");
		}
	}

	DataTexture2D.lodasatlas=false;
	__static(DataTexture2D,
	['simLodRect',function(){return this.simLodRect=new Uint32Array([
		0,0,512,256,
		512,0,256,128,
		512+256,0,128,64,
		512+256+128,0,64,32,
		512+256+128+64,0,32,16,
		512+256+128+64+32,0,16,8,
		512+256+128+64+32+16,0,8,4,
		512+256+128+64+32+16+8,0,4,2,
		512+256+128+64+32+16+8+4,0,2,1,
		512+256+128+64+32+16+8+4+2,0,1,1]);}
	]);
	return DataTexture2D;
})(BaseTexture)


/**
*@private
*<code>PrimitiveMesh</code> 类用于创建基本网格的父类。
*/
//class laya.d3.resource.models.PrimitiveMesh extends laya.d3.resource.models.BaseMesh
var PrimitiveMesh=(function(_super){
	function PrimitiveMesh(){
		this._numberVertices=0;
		this._numberIndices=0;
		this._vertexBuffer=null;
		this._indexBuffer=null;
		PrimitiveMesh.__super.call(this);
	}

	__class(PrimitiveMesh,'laya.d3.resource.models.PrimitiveMesh',_super);
	var __proto=PrimitiveMesh.prototype;
	Laya.imps(__proto,{"laya.d3.core.render.IRenderable":true})
	__proto._getVertexBuffer=function(index){
		(index===void 0)&& (index=0);
		if (index===0)
			return this._vertexBuffer;
		else
		return null;
	}

	__proto._getVertexBuffers=function(){
		return null;
	}

	__proto._getIndexBuffer=function(){
		return this._indexBuffer;
	}

	/**
	*获取网格顶点
	*@return 网格顶点。
	*/
	__proto._getPositions=function(){
		var vertices=[];
		var positionElement;
		var vertexElements=this._vertexBuffer.vertexDeclaration.getVertexElements();
		var j=0;
		for (j=0;j < vertexElements.length;j++){
			var vertexElement=vertexElements[j];
			if (vertexElement.elementFormat===/*laya.d3.graphics.VertexElementFormat.Vector3*/"vector3" && vertexElement.elementUsage===/*laya.d3.graphics.VertexElementUsage.POSITION0*/0){
				positionElement=vertexElement;
				break ;
			}
		};
		var verticesData=this._vertexBuffer.getData();
		for (j=0;j < verticesData.length;j+=this._vertexBuffer.vertexDeclaration.vertexStride / 4){
			var ofset=j+positionElement.offset / 4;
			var position=new Vector3(verticesData[ofset+0],verticesData[ofset+1],verticesData[ofset+2]);
			vertices.push(position);
		}
		return vertices;
	}

	__proto.getRenderElement=function(index){
		return this;
	}

	__proto.getRenderElementsCount=function(){
		return 1;
	}

	__proto.disposeResource=function(){
		(this._vertexBuffer)&& (this._vertexBuffer.destroy(),this._vertexBuffer=null);
		(this._indexBuffer)&& (this._indexBuffer.destroy(),this._indexBuffer=null);
		this.memorySize=0;
	}

	__proto._beforeRender=function(state){
		this._vertexBuffer._bind();
		this._indexBuffer._bind();
		return true;
	}

	__proto._render=function(state){
		WebGL.mainContext.drawElements(/*laya.webgl.WebGLContext.TRIANGLES*/0x0004,this._numberIndices,/*laya.webgl.WebGLContext.UNSIGNED_SHORT*/0x1403,0);
		Stat.drawCall++;
		Stat.trianglesFaces+=this._numberIndices / 3;
	}

	__getset(0,__proto,'_vertexBufferCount',function(){
		return 1;
	});

	__getset(0,__proto,'triangleCount',function(){
		return this._indexBuffer.indexCount / 3;
	});

	return PrimitiveMesh;
})(BaseMesh)



/**
*<code>RenderTarget</code> 类用于创建渲染目标。
*/
//class laya.d3.resource.RenderTexture extends laya.d3.resource.BaseTexture
var RenderTexture=(function(_super){
	function RenderTexture(width,height,surfaceFormat,surfaceType,depthStencilFormat,mipMap,repeat,minFifter,magFifter){
		/**@private */
		this._alreadyResolved=false;
		/**@private */
		this._surfaceFormat=0;
		/**@private */
		this._surfaceType=0;
		/**@private */
		this._depthStencilFormat=0;
		/**@private */
		this._frameBuffer=null;
		/**@private */
		this._depthStencilBuffer=null;
		(surfaceFormat===void 0)&& (surfaceFormat=/*laya.webgl.WebGLContext.RGBA*/0x1908);
		(surfaceType===void 0)&& (surfaceType=/*laya.webgl.WebGLContext.UNSIGNED_BYTE*/0x1401);
		(depthStencilFormat===void 0)&& (depthStencilFormat=/*laya.webgl.WebGLContext.DEPTH_COMPONENT16*/0x81A5);
		(mipMap===void 0)&& (mipMap=false);
		(repeat===void 0)&& (repeat=false);
		(minFifter===void 0)&& (minFifter=-1);
		(magFifter===void 0)&& (magFifter=-1);
		RenderTexture.__super.call(this);
		this._type=/*laya.webgl.WebGLContext.TEXTURE_2D*/0x0DE1;
		this._width=width;
		this._height=height;
		this._size=new Size(width,height);
		this._surfaceFormat=surfaceFormat;
		this._surfaceType=surfaceType;
		this._depthStencilFormat=depthStencilFormat;
		this._mipmap=mipMap;
		this._repeat=repeat;
		this._minFifter=minFifter;
		this._magFifter=magFifter;
		this.activeResource();
		this._alreadyResolved=true;
	}

	__class(RenderTexture,'laya.d3.resource.RenderTexture',_super);
	var __proto=RenderTexture.prototype;
	__proto.recreateResource=function(){
		var gl=WebGL.mainContext;
		this._frameBuffer=gl.createFramebuffer();
		this._source=gl.createTexture();
		var preTarget=WebGLContext.curBindTexTarget;
		var preTexture=WebGLContext.curBindTexValue;
		WebGLContext.bindTexture(gl,this._type,this._source);
		gl.texImage2D(this._type,0,/*laya.webgl.WebGLContext.RGBA*/0x1908,this._width,this._height,0,this._surfaceFormat,this._surfaceType,null);
		var minFifter=this._minFifter;
		var magFifter=this._magFifter;
		var repeat=this._repeat ? /*laya.webgl.WebGLContext.REPEAT*/0x2901 :/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F;
		var isPot=Arith.isPOT(this._width,this._height);
		if (isPot){
			if (this._mipmap)
				(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR_MIPMAP_LINEAR*/0x2703);
			else
			(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			(magFifter!==-1)|| (magFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MIN_FILTER*/0x2801,minFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MAG_FILTER*/0x2800,magFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_S*/0x2802,repeat);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_T*/0x2803,repeat);
			this._mipmap && gl.generateMipmap(this._type);
			}else {
			(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			(magFifter!==-1)|| (magFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MIN_FILTER*/0x2801,minFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MAG_FILTER*/0x2800,magFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_S*/0x2802,/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_T*/0x2803,/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F);
		}
		gl.bindFramebuffer(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,this._frameBuffer);
		gl.framebufferTexture2D(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,/*laya.webgl.WebGLContext.COLOR_ATTACHMENT0*/0x8CE0,/*laya.webgl.WebGLContext.TEXTURE_2D*/0x0DE1,this._source,0);
		if (this._depthStencilFormat){
			this._depthStencilBuffer=gl.createRenderbuffer();
			gl.bindRenderbuffer(/*laya.webgl.WebGLContext.RENDERBUFFER*/0x8D41,this._depthStencilBuffer);
			gl.renderbufferStorage(/*laya.webgl.WebGLContext.RENDERBUFFER*/0x8D41,this._depthStencilFormat,this._width,this._height);
			switch (this._depthStencilFormat){
				case /*laya.webgl.WebGLContext.DEPTH_COMPONENT16*/0x81A5:
					gl.framebufferRenderbuffer(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,/*laya.webgl.WebGLContext.DEPTH_ATTACHMENT*/0x8D00,/*laya.webgl.WebGLContext.RENDERBUFFER*/0x8D41,this._depthStencilBuffer);
					break ;
				case /*laya.webgl.WebGLContext.STENCIL_INDEX8*/0x8D48:
					gl.framebufferRenderbuffer(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,/*laya.webgl.WebGLContext.STENCIL_ATTACHMENT*/0x8D20,/*laya.webgl.WebGLContext.RENDERBUFFER*/0x8D41,this._depthStencilBuffer);
					break ;
				case /*laya.webgl.WebGLContext.DEPTH_STENCIL*/0x84F9:
					gl.framebufferRenderbuffer(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,/*laya.webgl.WebGLContext.DEPTH_STENCIL_ATTACHMENT*/0x821A,/*laya.webgl.WebGLContext.RENDERBUFFER*/0x8D41,this._depthStencilBuffer);
					break ;
				}
		}
		gl.bindFramebuffer(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,null);
		(preTarget && preTexture)&& (WebGLContext.bindTexture(gl,preTarget,preTexture));
		gl.bindRenderbuffer(/*laya.webgl.WebGLContext.RENDERBUFFER*/0x8D41,null);
		this.memorySize=this._width *this._height *4;
		this.completeCreate();
	}

	/**
	*开始绑定。
	*/
	__proto.start=function(){
		WebGL.mainContext.bindFramebuffer(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,this.frameBuffer);
		RenderTexture._currentRenderTarget=this;
		this._alreadyResolved=false;
	}

	/**
	*结束绑定。
	*/
	__proto.end=function(){
		WebGL.mainContext.bindFramebuffer(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,null);
		RenderTexture._currentRenderTarget=null;
		this._alreadyResolved=true;
	}

	/**
	*获得像素数据。
	*@param x X像素坐标。
	*@param y Y像素坐标。
	*@param width 宽度。
	*@param height 高度。
	*@return 像素数据。
	*/
	__proto.getData=function(x,y,width,height){
		var gl=WebGL.mainContext;
		gl.bindFramebuffer(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,this._frameBuffer);
		var canRead=(gl.checkFramebufferStatus(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40)===/*laya.webgl.WebGLContext.FRAMEBUFFER_COMPLETE*/0x8CD5);
		if (!canRead){
			gl.bindFramebuffer(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,null);
			return null;
		};
		var pixels=new Uint8Array(this._width *this._height *4);
		gl.readPixels(x,y,width,height,this._surfaceFormat,this._surfaceType,pixels);
		gl.bindFramebuffer(/*laya.webgl.WebGLContext.FRAMEBUFFER*/0x8D40,null);
		return pixels;
	}

	/**
	*销毁资源。
	*/
	__proto.disposeResource=function(){
		if (this._frameBuffer){
			var gl=WebGL.mainContext;
			gl.deleteTexture(this._source);
			gl.deleteFramebuffer(this._frameBuffer);
			gl.deleteRenderbuffer(this._depthStencilBuffer);
			this._source=null;
			this._frameBuffer=null;
			this._depthStencilBuffer=null;
			this.memorySize=0;
		}
	}

	/**
	*获取表面格式。
	*@return 表面格式。
	*/
	__getset(0,__proto,'surfaceFormat',function(){
		return this._surfaceFormat;
	});

	/**
	*获取表面类型。
	*@return 表面类型。
	*/
	__getset(0,__proto,'surfaceType',function(){
		return this._surfaceType;
	});

	/**
	*获取深度格式。
	*@return 深度格式。
	*/
	__getset(0,__proto,'depthStencilFormat',function(){
		return this._depthStencilFormat;
	});

	/**
	*获取RenderTarget数据源,如果alreadyResolved等于false，则返回null。
	*@return RenderTarget数据源。
	*/
	__getset(0,__proto,'source',function(){
		if (this._alreadyResolved)
			return Laya.superGet(BaseTexture,this,'source');
		else
		return null;
	});

	__getset(0,__proto,'depthStencilBuffer',function(){
		return this._depthStencilBuffer;
	});

	__getset(0,__proto,'frameBuffer',function(){
		return this._frameBuffer;
	});

	RenderTexture._currentRenderTarget=null;
	return RenderTexture;
})(BaseTexture)


/**
*<code>SolidColorTexture2D</code> 二维纯色纹理。
*/
//class laya.d3.resource.SolidColorTexture2D extends laya.d3.resource.BaseTexture
var SolidColorTexture2D=(function(_super){
	function SolidColorTexture2D(color){
		/**@private */
		this._color=null;
		/**@private */
		this._pixels=null;
		SolidColorTexture2D.__super.call(this);
		this._type=/*laya.webgl.WebGLContext.TEXTURE_2D*/0x0DE1;
		this._width=1;
		this._height=1;
		this._size=new Size(this.width,this.height);
		this._color=color;
		this._pixels=new Uint8Array([color.x *255,color.y *255,color.z *255,color.w *255]);
	}

	__class(SolidColorTexture2D,'laya.d3.resource.SolidColorTexture2D',_super);
	var __proto=SolidColorTexture2D.prototype;
	/**
	*@private
	*/
	__proto._createWebGlTexture=function(){
		var gl=WebGL.mainContext;
		var glTex=this._source=gl.createTexture();
		var w=this._width;
		var h=this._height;
		var preTarget=WebGLContext.curBindTexTarget;
		var preTexture=WebGLContext.curBindTexValue;
		WebGLContext.bindTexture(gl,this._type,glTex);
		gl.texImage2D(this._type,0,/*laya.webgl.WebGLContext.RGBA*/0x1908,w,h,0,/*laya.webgl.WebGLContext.RGBA*/0x1908,/*laya.webgl.WebGLContext.UNSIGNED_BYTE*/0x1401,this._pixels);
		var minFifter=this._minFifter;
		var magFifter=this._magFifter;
		var repeat=this._repeat ? /*laya.webgl.WebGLContext.REPEAT*/0x2901 :/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F;
		var isPot=Arith.isPOT(w,h);
		if (isPot){
			if (this._mipmap)
				(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR_MIPMAP_LINEAR*/0x2703);
			else
			(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			(magFifter!==-1)|| (magFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MIN_FILTER*/0x2801,minFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MAG_FILTER*/0x2800,magFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_S*/0x2802,repeat);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_T*/0x2803,repeat);
			this._mipmap && gl.generateMipmap(this._type);
			}else {
			(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			(magFifter!==-1)|| (magFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MIN_FILTER*/0x2801,minFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MAG_FILTER*/0x2800,magFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_S*/0x2802,/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_T*/0x2803,/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F);
		}
		(preTarget && preTexture)&& (WebGLContext.bindTexture(gl,preTarget,preTexture));
		if (isPot)
			this.memorySize=w *h *4 *(1+1 / 3);
		else
		this.memorySize=w *h *4;
	}

	/**
	*重新创建资源，如果异步创建中被强制释放再创建，则需等待释放完成后再重新加载创建。
	*/
	__proto.recreateResource=function(){
		this._createWebGlTexture();
		this.completeCreate();
	}

	/**
	*销毁资源。
	*/
	__proto.disposeResource=function(){
		if (this._source){
			WebGL.mainContext.deleteTexture(this._source);
			this._source=null;
			this.memorySize=0;
		}
	}

	__static(SolidColorTexture2D,
	['magentaTexture',function(){return this.magentaTexture=new SolidColorTexture2D(new Vector4(1.0,0.0,1.0,1.0));},'grayTexture',function(){return this.grayTexture=new SolidColorTexture2D(new Vector4(0.5,0.5,0.5,1.0));}
	]);
	return SolidColorTexture2D;
})(BaseTexture)

/**
*<code>Texture2D</code> 二维纹理。
*/
//class laya.d3.resource.Texture2D extends laya.d3.resource.BaseTexture
var Texture2D=(function(_super){
	function Texture2D(canRead,reapeat,format,mipmap){
		/**@private */
		this._canRead=false;
		/**@private HTML Image*/
		this._image=null;
		/**@private */
		this._pixels=null;
		(canRead===void 0)&& (canRead=false);
		(reapeat===void 0)&& (reapeat=true);
		(format===void 0)&& (format=/*laya.webgl.WebGLContext.RGBA*/0x1908);
		(mipmap===void 0)&& (mipmap=true);
		Texture2D.__super.call(this);
		this._type=/*laya.webgl.WebGLContext.TEXTURE_2D*/0x0DE1;
		this._repeat=reapeat;
		this._canRead=canRead;
		this._format=format;
		this._mipmap=mipmap;
	}

	__class(Texture2D,'laya.d3.resource.Texture2D',_super);
	var __proto=Texture2D.prototype;
	/**
	*@private
	*/
	__proto._createWebGlTexture=function(){
		if (!this._image)
			throw "create GLTextur err:no data:"+this._image;
		var gl=WebGL.mainContext;
		var glTexture=this._source=gl.createTexture();
		var w=this._width;
		var h=this._height;
		var preTarget=WebGLContext.curBindTexTarget;
		var preTexture=WebGLContext.curBindTexValue;
		WebGLContext.bindTexture(gl,this._type,glTexture);
		switch (this._format){
			case /*laya.webgl.WebGLContext.RGB*/0x1907:
			case /*laya.webgl.WebGLContext.RGBA*/0x1908:
				if (this._canRead)
					gl.texImage2D(this._type,0,this._format,w,h,0,this._format,/*laya.webgl.WebGLContext.UNSIGNED_BYTE*/0x1401,this._pixels);
				else
				gl.texImage2D(this._type,0,this._format,this._format,/*laya.webgl.WebGLContext.UNSIGNED_BYTE*/0x1401,this._image);
				break ;
			case WebGL.compressEtc1.COMPRESSED_RGB_ETC1_WEBGL:
				gl.compressedTexImage2D(this._type,0,this._format,this._width,this._height,0,this._image);
				break ;
			};
		var minFifter=this._minFifter;
		var magFifter=this._magFifter;
		var repeat=this._repeat ? /*laya.webgl.WebGLContext.REPEAT*/0x2901 :/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F;
		var isPot=Arith.isPOT(w,h);
		if (isPot){
			if (this._mipmap)
				(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR_MIPMAP_LINEAR*/0x2703);
			else
			(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			(magFifter!==-1)|| (magFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MIN_FILTER*/0x2801,minFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MAG_FILTER*/0x2800,magFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_S*/0x2802,repeat);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_T*/0x2803,repeat);
			this._mipmap && gl.generateMipmap(this._type);
			}else {
			(minFifter!==-1)|| (minFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			(magFifter!==-1)|| (magFifter=/*laya.webgl.WebGLContext.LINEAR*/0x2601);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MIN_FILTER*/0x2801,minFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_MAG_FILTER*/0x2800,magFifter);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_S*/0x2802,/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F);
			gl.texParameteri(this._type,/*laya.webgl.WebGLContext.TEXTURE_WRAP_T*/0x2803,/*laya.webgl.WebGLContext.CLAMP_TO_EDGE*/0x812F);
		}
		(preTarget && preTexture)&& (WebGLContext.bindTexture(gl,preTarget,preTexture));
		this._image.onload=null;
		this._image=null;
		if (isPot)
			this.memorySize=w *h *4 *(1+1 / 3);
		else
		this.memorySize=w *h *4;
	}

	/**
	*重新创建资源，如果异步创建中被强制释放再创建，则需等待释放完成后再重新加载创建。
	*/
	__proto.recreateResource=function(){
		this._createWebGlTexture();
		this.completeCreate();
	}

	/**
	*@private
	*/
	__proto.onAsynLoaded=function(url,data,params){
		if (params){
			var canRead=params[0];
			(canRead!==undefined)&& (this._canRead=canRead);
			var repeat=params[1];
			(repeat!==undefined)&& (this._repeat=repeat);
			var format=params[2];
			(format!==undefined)&& (this._format=format);
			var mipmap=params[3];
			(mipmap!==undefined)&& (this._mipmap=mipmap);
		}
		switch (this._format){
			case /*laya.webgl.WebGLContext.RGB*/0x1907:
			case /*laya.webgl.WebGLContext.RGBA*/0x1908:
				this._image=data;
				var w=data.width;
				var h=data.height;
				this._width=w;
				this._height=h;
				this._size=new Size(w,h);
				if (this._canRead){
					if (Render.isConchApp){
						if (/*__JS__ */data instanceof window.HTMLElement){
							this._pixels=new Uint8Array(data.getImageData(0,0,w,h));
						}
					}
					else {
						Browser.canvas.size(w,h);
						Browser.canvas.clear();
						Browser.context.drawImage(data,0,0,w,h);
						this._pixels=new Uint8Array(Browser.context.getImageData(0,0,w,h).data.buffer);
					}
				}
				break ;
			case WebGL.compressEtc1.COMPRESSED_RGB_ETC1_WEBGL:;
				var readData=new Byte(data);
				var magicNumber=readData.readUTFBytes(4);
				var version=readData.readUTFBytes(2);
				var dataType=readData.getInt16();
				readData.endian=/*laya.utils.Byte.BIG_ENDIAN*/"bigEndian";
				this._width=readData.getInt16();
				this._height=readData.getInt16();
				this._size=new Size(this._width,this._height);
				var originalWidth=readData.getInt16();
				var originalHeight=readData.getInt16();
				this._image=new Uint8Array(data,readData.pos);
			}
		this.recreateResource();
		this._endLoaded();
	}

	/**
	*返回图片像素。
	*@return 图片像素。
	*/
	__proto.getPixels=function(){
		if (this._canRead)
			return this._pixels;
		else
		throw new Error("Texture2D: must set texture canRead is true.");
	}

	/**
	*销毁资源。
	*/
	__proto.disposeResource=function(){
		if (this._source){
			WebGL.mainContext.deleteTexture(this._source);
			this._source=null;
			this._image=null;
			this.memorySize=0;
		}
	}

	/**@private */
	__getset(0,__proto,'_src',function(){
		return this.url;
	});

	/**@private */
	__getset(0,__proto,'src',function(){
		return this.url;
	});

	Texture2D.load=function(url,complete){
		return Laya.loader.create(url,complete,null,Texture2D);
	}

	return Texture2D;
})(BaseTexture)

/**
*<code>BaseCamera</code> 类用于创建摄像机的父类。
*/
//class laya.d3.core.BaseCamera extends laya.d3.core.Sprite3D
var BaseCamera=(function(_super){
	function BaseCamera(nearPlane,farPlane){
		/**@private */
		//this._tempVector3=null;
		/**@private 位置。*/
		//this._position=null;
		/**@private 向上向量。*/
		//this._up=null;
		/**@private 前向量。*/
		//this._forward=null;
		/**@private 右向量。*/
		//this._right=null;
		/**@private 渲染顺序。*/
		//this._renderingOrder=0;
		/**@private 渲染目标尺寸。*/
		//this._renderTargetSize=null;
		/**@private 近裁剪面。*/
		//this._nearPlane=NaN;
		/**@private 远裁剪面。*/
		//this._farPlane=NaN;
		/**@private 视野。*/
		//this._fieldOfView=NaN;
		/**@private 正交投影的垂直尺寸。*/
		//this._orthographicVerticalSize=NaN;
		/**@private 天空。*/
		//this._sky=null;
		/**@private */
		//this._orthographic=false;
		/**@private 渲染目标。*/
		//this._renderTarget=null;
		/**@private 是否使用用户自定义投影矩阵，如果使用了用户投影矩阵，摄像机投影矩阵相关的参数改变则不改变投影矩阵的值，需调用ResetProjectionMatrix方法。*/
		//this._useUserProjectionMatrix=false;
		/**@private 表明视口是否使用裁剪空间表达。*/
		//this._viewportExpressedInClipSpace=false;
		/**清楚标记。*/
		//this.clearFlag=0;
		/**摄像机的清除颜色。*/
		//this.clearColor=null;
		/**可视遮罩图层。 */
		//this.cullingMask=0;
		/**渲染时是否用遮挡剔除。 */
		//this.useOcclusionCulling=false;
		BaseCamera.__super.call(this);
		(nearPlane===void 0)&& (nearPlane=0.3);
		(farPlane===void 0)&& (farPlane=1000);
		this._tempVector3=new Vector3();
		this._position=new Vector3();
		this._up=new Vector3();
		this._forward=new Vector3();
		this._right=new Vector3();
		this._fieldOfView=60;
		this._useUserProjectionMatrix=false;
		this._orthographic=false;
		this._viewportExpressedInClipSpace=true;
		this._renderTargetSize=Size.fullScreen;
		this._orthographicVerticalSize=10;
		this.renderingOrder=0;
		this._nearPlane=nearPlane;
		this._farPlane=farPlane;
		this.cullingMask=2147483647;
		this.clearFlag=/*CLASS CONST:laya.d3.core.BaseCamera.CLEARFLAG_SOLIDCOLOR*/0;
		this.useOcclusionCulling=true;
		this._calculateProjectionMatrix();
		Laya.stage.on(/*laya.events.Event.RESIZE*/"resize",this,this._onScreenSizeChanged);
	}

	__class(BaseCamera,'laya.d3.core.BaseCamera',_super);
	var __proto=BaseCamera.prototype;
	/**
	*通过RenderingOrder属性对摄像机机型排序。
	*/
	__proto._sortCamerasByRenderingOrder=function(){
		if (this._displayedInStage){
			var cameraPool=this.scene._cameraPool;
			var n=cameraPool.length-1;
			for (var i=0;i < n;i++){
				if (cameraPool[i].renderingOrder > cameraPool[n].renderingOrder){
					var tempCamera=cameraPool[i];
					cameraPool[i]=cameraPool[n];
					cameraPool[n]=tempCamera;
				}
			}
		}
	}

	/**
	*@private
	*/
	__proto._calculateProjectionMatrix=function(){}
	/**
	*@private
	*/
	__proto._onScreenSizeChanged=function(){
		this._calculateProjectionMatrix();
	}

	/**
	*@private
	*/
	__proto._prepareCameraToRender=function(){
		Layer._currentCameraCullingMask=this.cullingMask;
		var cameraSV=this._shaderValues;
		cameraSV.setValue(/*CLASS CONST:laya.d3.core.BaseCamera.CAMERAPOS*/0,this.transform.position.elements);
		cameraSV.setValue(/*CLASS CONST:laya.d3.core.BaseCamera.CAMERADIRECTION*/5,this.forward.elements);
		cameraSV.setValue(/*CLASS CONST:laya.d3.core.BaseCamera.CAMERAUP*/6,this.up.elements);
	}

	/**
	*@private
	*/
	__proto._prepareCameraViewProject=function(viewMatrix,projectMatrix){
		var cameraSV=this._shaderValues;
		cameraSV.setValue(/*CLASS CONST:laya.d3.core.BaseCamera.VIEWMATRIX*/1,viewMatrix.elements);
		cameraSV.setValue(/*CLASS CONST:laya.d3.core.BaseCamera.PROJECTMATRIX*/2,projectMatrix.elements);
	}

	/**
	*@private
	*/
	__proto._renderCamera=function(gl,state,scene){}
	/**
	*增加可视图层。
	*@param layer 图层。
	*/
	__proto.addLayer=function(layer){
		if (layer.number===29 || layer.number==30)
			return;
		this.cullingMask=this.cullingMask | layer.mask;
	}

	/**
	*移除可视图层。
	*@param layer 图层。
	*/
	__proto.removeLayer=function(layer){
		if (layer.number===29 || layer.number==30)
			return;
		this.cullingMask=this.cullingMask & ~layer.mask;
	}

	/**
	*增加所有图层。
	*/
	__proto.addAllLayers=function(){
		this.cullingMask=2147483647;
	}

	/**
	*移除所有图层。
	*/
	__proto.removeAllLayers=function(){
		this.cullingMask=0 | Layer.getLayerByNumber(29).mask | Layer.getLayerByNumber(30).mask;
	}

	__proto.ResetProjectionMatrix=function(){
		this._useUserProjectionMatrix=false;
		this._calculateProjectionMatrix();
	}

	/**
	*向前移动。
	*@param distance 移动距离。
	*/
	__proto.moveForward=function(distance){
		this._tempVector3.elements[0]=this._tempVector3.elements[1]=0;
		this._tempVector3.elements[2]=distance;
		this.transform.translate(this._tempVector3);
	}

	/**
	*向右移动。
	*@param distance 移动距离。
	*/
	__proto.moveRight=function(distance){
		this._tempVector3.elements[1]=this._tempVector3.elements[2]=0;
		this._tempVector3.elements[0]=distance;
		this.transform.translate(this._tempVector3);
	}

	/**
	*向上移动。
	*@param distance 移动距离。
	*/
	__proto.moveVertical=function(distance){
		this._tempVector3.elements[0]=this._tempVector3.elements[2]=0;
		this._tempVector3.elements[1]=distance;
		this.transform.translate(this._tempVector3,false);
	}

	//}// BoundingFrustumWorldSpace
	__proto._addSelfRenderObjects=function(){
		var cameraPool=this.scene._cameraPool;
		var cmaeraCount=cameraPool.length;
		if (cmaeraCount > 0){
			for (var i=cmaeraCount-1;i >=0;i--){
				if (this.renderingOrder <=cameraPool[i].renderingOrder){
					cameraPool.splice(i+1,0,this);
					break ;
				}
			}
			}else {
			cameraPool.push(this);
			if (this.scene.conchModel){
				this.scene.conchModel.setCurrentCamera(this.conchModel);
			}
		}
	}

	__proto._clearSelfRenderObjects=function(){
		var cameraPool=this.scene._cameraPool;
		cameraPool.splice(cameraPool.indexOf(this),1);
	}

	/**
	*@inheritDoc
	*/
	__proto.destroy=function(destroyChild){
		(destroyChild===void 0)&& (destroyChild=true);
		(this._sky)&& (this._sky.destroy());
		this.renderTarget=null;
		Laya.stage.off(/*laya.events.Event.RESIZE*/"resize",this,this._onScreenSizeChanged);
		_super.prototype.destroy.call(this,destroyChild);
	}

	/**设置天空。*/
	/**获取天空。*/
	__getset(0,__proto,'sky',function(){
		return this._sky;
		},function(value){
		this._sky=value;
		value._ownerCamera=this;
	});

	/**
	*获取前向量。
	*@return 前向量。
	*/
	__getset(0,__proto,'forward',function(){
		var worldMatrixe=this.transform.worldMatrix.elements;
		var forwarde=this._forward.elements;
		forwarde[0]=-worldMatrixe[8];
		forwarde[1]=-worldMatrixe[9];
		forwarde[2]=-worldMatrixe[10];
		return this._forward;
	});

	/**获取位置。*/
	__getset(0,__proto,'position',function(){
		var worldMatrixe=this.transform.worldMatrix.elements;
		var positione=this._position.elements;
		positione[0]=worldMatrixe[12];
		positione[1]=worldMatrixe[13];
		positione[2]=worldMatrixe[14];
		return this._position;
	});

	/**
	*设置渲染场景的渲染目标。
	*@param value 渲染场景的渲染目标。
	*/
	/**
	*获取渲染场景的渲染目标。
	*@return 渲染场景的渲染目标。
	*/
	__getset(0,__proto,'renderTarget',function(){
		return this._renderTarget;
		},function(value){
		this._renderTarget=value;
		if (value !=null)
			this._renderTargetSize=value.size;
	});

	/**
	*获取上向量。
	*@return 上向量。
	*/
	__getset(0,__proto,'up',function(){
		var worldMatrixe=this.transform.worldMatrix.elements;
		var upe=this._up.elements;
		upe[0]=worldMatrixe[4];
		upe[1]=worldMatrixe[5];
		upe[2]=worldMatrixe[6];
		return this._up;
	});

	/**
	*获取右向量。
	*@return 右向量。
	*/
	__getset(0,__proto,'right',function(){
		var worldMatrixe=this.transform.worldMatrix.elements;
		var righte=this._right.elements;
		righte[0]=worldMatrixe[0];
		righte[1]=worldMatrixe[1];
		righte[2]=worldMatrixe[2];
		return this._right;
	});

	/**
	*设置渲染目标的尺寸
	*@param value 渲染目标的尺寸。
	*/
	/**
	*获取渲染目标的尺寸
	*@return 渲染目标的尺寸。
	*/
	__getset(0,__proto,'renderTargetSize',function(){
		return this._renderTargetSize;
		},function(value){
		if (this.renderTarget !=null && this._renderTargetSize !=value){}
			this._renderTargetSize=value;
		this._calculateProjectionMatrix();
	});

	/**
	*设置视野。
	*@param value 视野。
	*/
	/**
	*获取视野。
	*@return 视野。
	*/
	__getset(0,__proto,'fieldOfView',function(){
		return this._fieldOfView;
		},function(value){
		this._fieldOfView=value;
		this._calculateProjectionMatrix();
	});

	/**
	*设置近裁面。
	*@param value 近裁面。
	*/
	/**
	*获取近裁面。
	*@return 近裁面。
	*/
	__getset(0,__proto,'nearPlane',function(){
		return this._nearPlane;
		},function(value){
		this._nearPlane=value;
		this._calculateProjectionMatrix();
	});

	/**
	*设置远裁面。
	*@param value 远裁面。
	*/
	/**
	*获取远裁面。
	*@return 远裁面。
	*/
	__getset(0,__proto,'farPlane',function(){
		return this._farPlane;
		},function(vaule){
		this._farPlane=vaule;
		this._calculateProjectionMatrix();
	});

	/**
	*设置是否正交投影矩阵。
	*@param 是否正交投影矩阵。
	*/
	/**
	*获取是否正交投影矩阵。
	*@return 是否正交投影矩阵。
	*/
	__getset(0,__proto,'orthographic',function(){
		return this._orthographic;
		},function(vaule){
		this._orthographic=vaule;
		this._calculateProjectionMatrix();
	});

	/**
	*设置正交投影垂直矩阵尺寸。
	*@param 正交投影垂直矩阵尺寸。
	*/
	/**
	*获取正交投影垂直矩阵尺寸。
	*@return 正交投影垂直矩阵尺寸。
	*/
	__getset(0,__proto,'orthographicVerticalSize',function(){
		return this._orthographicVerticalSize;
		},function(vaule){
		this._orthographicVerticalSize=vaule;
		this._calculateProjectionMatrix();
	});

	__getset(0,__proto,'renderingOrder',function(){
		return this._renderingOrder;
		},function(value){
		this._renderingOrder=value;
		this._sortCamerasByRenderingOrder();
	});

	BaseCamera.CAMERAPOS=0;
	BaseCamera.VIEWMATRIX=1;
	BaseCamera.PROJECTMATRIX=2;
	BaseCamera.VPMATRIX=3;
	BaseCamera.VPMATRIX_NO_TRANSLATE=4;
	BaseCamera.CAMERADIRECTION=5;
	BaseCamera.CAMERAUP=6;
	BaseCamera.ENVIRONMENTDIFFUSE=7;
	BaseCamera.ENVIRONMENTSPECULAR=8;
	BaseCamera.SIMLODINFO=9;
	BaseCamera.DIFFUSEIRRADMATR=10;
	BaseCamera.DIFFUSEIRRADMATG=11;
	BaseCamera.DIFFUSEIRRADMATB=12;
	BaseCamera.HDREXPOSURE=13;
	BaseCamera.RENDERINGTYPE_DEFERREDLIGHTING="DEFERREDLIGHTING";
	BaseCamera.RENDERINGTYPE_FORWARDRENDERING="FORWARDRENDERING";
	BaseCamera.CLEARFLAG_SOLIDCOLOR=0;
	BaseCamera.CLEARFLAG_SKY=1;
	BaseCamera.CLEARFLAG_DEPTHONLY=2;
	BaseCamera.CLEARFLAG_NONE=3;
	__static(BaseCamera,
	['_invertYScaleMatrix',function(){return this._invertYScaleMatrix=new Matrix4x4(1,0,0,0,0,-1,0,0,0,0,1,0,0,0,0,1);},'_invertYProjectionMatrix',function(){return this._invertYProjectionMatrix=new Matrix4x4();},'_invertYProjectionViewMatrix',function(){return this._invertYProjectionViewMatrix=new Matrix4x4();}
	]);
	return BaseCamera;
})(Sprite3D)


/**
*<code>RenderableSprite3D</code> 类用于可渲染3D精灵的父类，抽象类不允许实例。
*/
//class laya.d3.core.RenderableSprite3D extends laya.d3.core.Sprite3D
var RenderableSprite3D=(function(_super){
	function RenderableSprite3D(name){
		/**@private */
		this._render=null;
		/**@private */
		this._geometryFilter=null;
		RenderableSprite3D.__super.call(this,name)
	}

	__class(RenderableSprite3D,'laya.d3.core.RenderableSprite3D',_super);
	var __proto=RenderableSprite3D.prototype;
	/**
	*@private
	*/
	__proto._addToInitStaticBatchManager=function(){}
	/**
	*@inheritDoc
	*/
	__proto._setBelongScene=function(scene){
		_super.prototype._setBelongScene.call(this,scene);
		scene._renderableSprite3Ds.push(this);
		this._render._applyLightMapParams();
	}

	/**
	*@inheritDoc
	*/
	__proto._setUnBelongScene=function(){
		var renderableSprite3Ds=this._scene._renderableSprite3Ds;
		var index=renderableSprite3Ds.indexOf(this);
		renderableSprite3Ds.splice(index,1);
		this._render._removeShaderDefine(laya.d3.core.RenderableSprite3D.SAHDERDEFINE_LIGHTMAP);
		_super.prototype._setUnBelongScene.call(this);
	}

	/**
	*@inheritDoc
	*/
	__proto._update=function(state){
		state.owner=this;
		if (this._activeInHierarchy){
			this._updateComponents(state);
			this._render._updateOctreeNode();
			this._lateUpdateComponents(state);
			Stat.spriteCount++;
			this._childs.length && this._updateChilds(state);
		}
	}

	/**
	*@inheritDoc
	*/
	__proto.destroy=function(destroyChild){
		(destroyChild===void 0)&& (destroyChild=true);
		_super.prototype.destroy.call(this,destroyChild);
		this._render._destroy();
		this._render=null;
	}

	RenderableSprite3D.__init__=function(){
		RenderableSprite3D.SHADERDEFINE_SCALEOFFSETLIGHTINGMAPUV=RenderableSprite3D.shaderDefines.registerDefine("SCALEOFFSETLIGHTINGMAPUV");
		RenderableSprite3D.SAHDERDEFINE_LIGHTMAP=RenderableSprite3D.shaderDefines.registerDefine("LIGHTMAP");
	}

	RenderableSprite3D.SHADERDEFINE_SCALEOFFSETLIGHTINGMAPUV=0x2;
	RenderableSprite3D.SAHDERDEFINE_LIGHTMAP=0x4;
	RenderableSprite3D.LIGHTMAPSCALEOFFSET=2;
	RenderableSprite3D.LIGHTMAP=3;
	__static(RenderableSprite3D,
	['shaderDefines',function(){return this.shaderDefines=new ShaderDefines$1();}
	]);
	return RenderableSprite3D;
})(Sprite3D)



/**
*<code>Camera</code> 类用于创建摄像机。
*/
//class laya.d3.core.Camera extends laya.d3.core.BaseCamera
var Camera=(function(_super){
	function Camera(aspectRatio,nearPlane,farPlane){
		/**@private */
		//this._aspectRatio=NaN;
		/**@private */
		//this._viewport=null;
		/**@private */
		//this._normalizedViewport=null;
		/**@private */
		//this._viewMatrix=null;
		/**@private */
		//this._projectionMatrix=null;
		/**@private */
		//this._projectionViewMatrix=null;
		/**@private */
		//this._boundFrustumUpdate=false;
		/**@private */
		//this._boundFrustum=null;
		/**@private */
		//this._orientedBoundBox=null;
		(aspectRatio===void 0)&& (aspectRatio=0);
		(nearPlane===void 0)&& (nearPlane=0.3);
		(farPlane===void 0)&& (farPlane=1000);
		this._viewMatrix=new Matrix4x4();
		this._projectionMatrix=new Matrix4x4();
		this._projectionViewMatrix=new Matrix4x4();
		this._viewport=new Viewport(0,0,0,0);
		this._normalizedViewport=new Viewport(0,0,1,1);
		this._aspectRatio=aspectRatio;
		this._boundFrustumUpdate=true;
		this._boundFrustum=new BoundFrustum(Matrix4x4.DEFAULT);
		Camera.__super.call(this,nearPlane,farPlane);
		this.transform.on(/*laya.events.Event.WORLDMATRIX_NEEDCHANGE*/"worldmatrixneedchanged",this,this._onWorldMatrixChanged);
	}

	__class(Camera,'laya.d3.core.Camera',_super);
	var __proto=Camera.prototype;
	/**
	*@private
	*/
	__proto._onWorldMatrixChanged=function(){
		this._boundFrustumUpdate=true;
	}

	/**
	*@inheritDoc
	*/
	__proto._parseCustomProps=function(rootNode,innerResouMap,customProps,json){
		var color=customProps.clearColor;
		this.clearColor=new Vector4(color[0],color[1],color[2],color[3]);
		var viewport=customProps.viewport;
		this.normalizedViewport=new Viewport(viewport[0],viewport[1],viewport[2],viewport[3]);
	}

	/**
	*@inheritDoc
	*/
	__proto._calculateProjectionMatrix=function(){
		if (!this._useUserProjectionMatrix){
			if (this._orthographic){
				var halfWidth=this.orthographicVerticalSize *this.aspectRatio *0.5;
				var halfHeight=this.orthographicVerticalSize *0.5;
				Matrix4x4.createOrthoOffCenterRH(-halfWidth,halfWidth,-halfHeight,halfHeight,this.nearPlane,this.farPlane,this._projectionMatrix);
				}else {
				Matrix4x4.createPerspective(3.1416 *this.fieldOfView / 180.0,this.aspectRatio,this.nearPlane,this.farPlane,this._projectionMatrix);
			}
		}
		this._boundFrustumUpdate=true;
	}

	/**
	*@inheritDoc
	*/
	__proto._update=function(state){
		if (this.conchModel){
			this.conchModel.setViewMatrix(this.viewMatrix.elements);
			this.conchModel.setProjectMatrix(this.projectionMatrix.elements);
		}
		laya.d3.core.Sprite3D.prototype._update.call(this,state);
	}

	/**
	*@inheritDoc
	*/
	__proto._renderCamera=function(gl,state,scene){
		(scene.parallelSplitShadowMaps[0])&& (scene._renderShadowMap(gl,state,this));
		state.camera=this;
		this._prepareCameraToRender();
		scene._preRenderUpdateComponents(state);
		var viewMat,projectMat;
		viewMat=state._viewMatrix=this.viewMatrix;
		var renderTar=this._renderTarget;
		if (renderTar){
			renderTar.start();
			Matrix4x4.multiply(BaseCamera._invertYScaleMatrix,this._projectionMatrix,BaseCamera._invertYProjectionMatrix);
			Matrix4x4.multiply(BaseCamera._invertYScaleMatrix,this.projectionViewMatrix,BaseCamera._invertYProjectionViewMatrix);
			projectMat=state._projectionMatrix=BaseCamera._invertYProjectionMatrix;
			state._projectionViewMatrix=BaseCamera._invertYProjectionViewMatrix;
			}else {
			projectMat=state._projectionMatrix=this._projectionMatrix;
			state._projectionViewMatrix=this.projectionViewMatrix;
		}
		this._prepareCameraViewProject(viewMat,projectMat);
		state._viewport=this.viewport;
		scene._preRenderScene(gl,state,this.boundFrustum);
		scene._clear(gl,state);
		scene._renderScene(gl,state);
		scene._postRenderUpdateComponents(state);
		(renderTar)&& (renderTar.end());
	}

	/**
	*计算从屏幕空间生成的射线。
	*@param point 屏幕空间的位置位置。
	*@return out 输出射线。
	*/
	__proto.viewportPointToRay=function(point,out){
		Picker.calculateCursorRay(point,this.viewport,this._projectionMatrix,this.viewMatrix,null,out);
	}

	/**
	*计算从裁切空间生成的射线。
	*@param point 裁切空间的位置。。
	*@return out 输出射线。
	*/
	__proto.normalizedViewportPointToRay=function(point,out){
		var finalPoint=Camera._tempVector20;
		var vp=this.viewport;
		var nVpPosE=point.elements;
		var vpPosE=finalPoint.elements;
		vpPosE[0]=nVpPosE[0] *vp.width;
		vpPosE[1]=nVpPosE[1] *vp.height;
		Picker.calculateCursorRay(finalPoint,this.viewport,this._projectionMatrix,this.viewMatrix,null,out);
	}

	/**
	*计算从世界空间准换三维坐标到屏幕空间。
	*@param position 世界空间的位置。
	*@return out 输出位置。
	*/
	__proto.worldToViewportPoint=function(position,out){
		Matrix4x4.multiply(this._projectionMatrix,this._viewMatrix,this._projectionViewMatrix);
		this.viewport.project(position,this._projectionViewMatrix,out);
		var outE=out.elements;
		outE[0]=outE[0] / Laya.stage.clientScaleX;
		outE[1]=outE[1] / Laya.stage.clientScaleY;
	}

	/**
	*计算从世界空间准换三维坐标到裁切空间。
	*@param position 世界空间的位置。
	*@return out 输出位置。
	*/
	__proto.worldToNormalizedViewportPoint=function(position,out){
		Matrix4x4.multiply(this._projectionMatrix,this._viewMatrix,this._projectionViewMatrix);
		this.normalizedViewport.project(position,this._projectionViewMatrix,out);
		var outE=out.elements;
		outE[0]=outE[0] / Laya.stage.clientScaleX;
		outE[1]=outE[1] / Laya.stage.clientScaleY;
	}

	/**
	*转换2D屏幕坐标系统到3D正交投影下的坐标系统，注:只有正交模型下有效。
	*@param source 源坐标。
	*@param out 输出坐标。
	*@return 是否转换成功。
	*/
	__proto.convertScreenCoordToOrthographicCoord=function(source,out){
		if (this._orthographic){
			var clientWidth=RenderState.clientWidth;
			var clientHeight=RenderState.clientHeight;
			var ratioX=this.orthographicVerticalSize *this.aspectRatio / clientWidth;
			var ratioY=this.orthographicVerticalSize / clientHeight;
			var sE=source.elements;
			var oE=out.elements;
			oE[0]=(-clientWidth / 2+sE[0])*ratioX;
			oE[1]=(clientHeight / 2-sE[1])*ratioY;
			oE[2]=(this.nearPlane-this.farPlane)*(sE[2]+1)/ 2-this.nearPlane;
			Vector3.transformCoordinate(out,this.transform.worldMatrix,out);
			return true;
			}else {
			return false;
		}
	}

	/**
	*获取视图投影矩阵。
	*@return 视图投影矩阵。
	*/
	__getset(0,__proto,'projectionViewMatrix',function(){
		Matrix4x4.multiply(this.projectionMatrix,this.viewMatrix,this._projectionViewMatrix);
		return this._projectionViewMatrix;
	});

	/**
	*设置横纵比。
	*@param value 横纵比。
	*/
	/**
	*获取横纵比。
	*@return 横纵比。
	*/
	__getset(0,__proto,'aspectRatio',function(){
		if (this._aspectRatio===0){
			var vp=this.viewport;
			return vp.width / vp.height;
		}
		return this._aspectRatio;
		},function(value){
		if (value < 0)
			throw new Error("Camera: the aspect ratio has to be a positive real number.");
		this._aspectRatio=value;
		this._calculateProjectionMatrix();
	});

	/**
	*获取摄像机视锥。
	*/
	__getset(0,__proto,'boundFrustum',function(){
		if (this._boundFrustumUpdate)
			this._boundFrustum.matrix=this.projectionViewMatrix;
		return this._boundFrustum;
	});

	__getset(0,__proto,'needViewport',function(){
		var nVp=this.normalizedViewport;
		return nVp.x===0 && nVp.y===0 && nVp.width===1 && nVp.height===1;
	});

	/**
	*设置屏幕空间的视口。
	*@param 屏幕空间的视口。
	*/
	/**
	*获取屏幕空间的视口。
	*@return 屏幕空间的视口。
	*/
	__getset(0,__proto,'viewport',function(){
		if (this._viewportExpressedInClipSpace){
			var nVp=this._normalizedViewport;
			var size=this.renderTargetSize;
			var sizeW=size.width;
			var sizeH=size.height;
			this._viewport.x=nVp.x *sizeW;
			this._viewport.y=nVp.y *sizeH;
			this._viewport.width=nVp.width *sizeW;
			this._viewport.height=nVp.height *sizeH;
		}
		return this._viewport;
		},function(value){
		if (this.renderTarget !=null && (value.x < 0 || value.y < 0 || value.width==0 || value.height==0))
			throw new Error("Camera: viewport size invalid.","value");
		this._viewportExpressedInClipSpace=false;
		this._viewport=value;
		this._calculateProjectionMatrix();
	});

	/**
	*设置裁剪空间的视口。
	*@return 裁剪空间的视口。
	*/
	/**
	*获取裁剪空间的视口。
	*@return 裁剪空间的视口。
	*/
	__getset(0,__proto,'normalizedViewport',function(){
		if (!this._viewportExpressedInClipSpace){
			var vp=this._viewport;
			var size=this.renderTargetSize;
			var sizeW=size.width;
			var sizeH=size.height;
			this._normalizedViewport.x=vp.x / sizeW;
			this._normalizedViewport.y=vp.y / sizeH;
			this._normalizedViewport.width=vp.width / sizeW;
			this._normalizedViewport.height=vp.height / sizeH;
		}
		return this._normalizedViewport;
		},function(value){
		if (value.x < 0){
			value.x=0;
			console.warn("Camera: viewport.x must large than 0.0.");
		}
		if (value.y < 0){
			value.y=0;
			console.warn("Camera: viewport.y must large than 0.0.");
		}
		if (value.x+value.width > 1.0){
			value.width=1.0-value.x;
			console.warn("Camera: viewport.width + viewport.x must less than 1.0.");
		}
		if ((value.y+value.height)> 1.0){
			value.height=1.0-value.y;
			console.warn("Camera: viewport.height + viewport.y must less than 1.0.");
		}
		this._viewportExpressedInClipSpace=true;
		this._normalizedViewport=value;
		this._calculateProjectionMatrix();
	});

	/**设置投影矩阵。*/
	/**获取投影矩阵。*/
	__getset(0,__proto,'projectionMatrix',function(){
		return this._projectionMatrix;
		},function(value){
		this._projectionMatrix=value;
		this._useUserProjectionMatrix=true;
	});

	/**
	*获取视图矩阵。
	*@return 视图矩阵。
	*/
	__getset(0,__proto,'viewMatrix',function(){
		this.transform.worldMatrix.invert(this._viewMatrix);
		return this._viewMatrix;
	});

	__static(Camera,
	['_tempVector20',function(){return this._tempVector20=new Vector2();}
	]);
	return Camera;
})(BaseCamera)


/**
*<code>MeshSprite3D</code> 类用于创建网格。
*/
//class laya.d3.core.MeshSprite3D extends laya.d3.core.RenderableSprite3D
var MeshSprite3D=(function(_super){
	/**
	*创建一个 <code>MeshSprite3D</code> 实例。
	*@param mesh 网格,同时会加载网格所用默认材质。
	*@param name 名字。
	*/
	function MeshSprite3D(mesh,name){
		MeshSprite3D.__super.call(this,name);
		this._geometryFilter=new MeshFilter(this);
		this._render=new MeshRender(this);
		this._geometryFilter.on(/*laya.events.Event.MESH_CHANGED*/"meshchanged",this,this._onMeshChanged);
		this._render.on(/*laya.events.Event.MATERIAL_CHANGED*/"materialchanged",this,this._onMaterialChanged);
		if (mesh){
			(this._geometryFilter).sharedMesh=mesh;
		}
	}

	__class(MeshSprite3D,'laya.d3.core.MeshSprite3D',_super);
	var __proto=MeshSprite3D.prototype;
	/**
	*@private
	*/
	__proto._changeRenderObjectByMesh=function(index){
		var renderObjects=this._render._renderElements;
		var renderElement=renderObjects[index];
		(renderElement)|| (renderElement=renderObjects[index]=new SubMeshRenderElement());
		renderElement._render=this._render;
		var material=this._render.sharedMaterials[index];
		(material)|| (material=StandardMaterial.defaultMaterial);
		var renderObj=(this._geometryFilter).sharedMesh.getRenderElement(index);
		renderElement._mainSortID=this._getSortID(renderObj,material);
		renderElement._sprite3D=this;
		renderElement.renderObj=renderObj;
		renderElement._material=material;
		return renderElement;
	}

	/**
	*@private
	*/
	__proto._changeRenderObjectByMaterial=function(index,material){
		var renderElement=this._render._renderElements[index];
		(material)|| (material=StandardMaterial.defaultMaterial);
		var renderObj=(this._geometryFilter).sharedMesh.getRenderElement(index);
		renderElement._mainSortID=this._getSortID(renderObj,material);
		renderElement._sprite3D=this;
		renderElement.renderObj=renderObj;
		renderElement._material=material;
		return renderElement;
	}

	/**
	*@private
	*/
	__proto._changeRenderObjectsByMesh=function(){
		var renderElementsCount=(this._geometryFilter).sharedMesh.getRenderElementsCount();
		this._render._renderElements.length=renderElementsCount;
		for (var i=0;i < renderElementsCount;i++)
		this._changeRenderObjectByMesh(i);
	}

	/**
	*@private
	*/
	__proto._onMeshChanged=function(meshFilter){
		var mesh=meshFilter.sharedMesh;
		if (mesh.loaded)
			this._changeRenderObjectsByMesh();
		else
		mesh.once(/*laya.events.Event.LOADED*/"loaded",this,this._onMeshLoaded);
	}

	/**
	*@private
	*/
	__proto._onMeshLoaded=function(sender){
		(sender===this.meshFilter.sharedMesh)&& (this._changeRenderObjectsByMesh());
	}

	/**
	*@private
	*/
	__proto._onMaterialChanged=function(meshRender,index,material){
		var renderElementCount=this._render._renderElements.length;
		(index < renderElementCount)&& this._changeRenderObjectByMaterial(index,material);
	}

	/**
	*@inheritDoc
	*/
	__proto._clearSelfRenderObjects=function(){
		this.scene.removeFrustumCullingObject(this._render);
	}

	/**
	*@inheritDoc
	*/
	__proto._addSelfRenderObjects=function(){
		this.scene.addFrustumCullingObject(this._render);
	}

	/**
	*@inheritDoc
	*/
	__proto._parseCustomProps=function(rootNode,innerResouMap,customProps,json){
		var render=this.meshRender;
		var lightmapIndex=customProps.lightmapIndex;
		(lightmapIndex !=null)&& (render.lightmapIndex=lightmapIndex);
		var lightmapScaleOffsetArray=customProps.lightmapScaleOffset;
		(lightmapScaleOffsetArray)&& (render.lightmapScaleOffset=new Vector4(lightmapScaleOffsetArray[0],lightmapScaleOffsetArray[1],lightmapScaleOffsetArray[2],lightmapScaleOffsetArray[3]));
		var meshPath,mesh;
		if (json.instanceParams){
			meshPath=json.instanceParams.loadPath;
			if (meshPath){
				mesh=Loader.getRes(innerResouMap[meshPath]);
				this.meshFilter.sharedMesh=mesh;
				if (mesh.loaded)
					render.sharedMaterials=mesh.materials;
				else
				mesh.once(/*laya.events.Event.LOADED*/"loaded",this,this._applyMeshMaterials);
			}
			}else {
			meshPath=customProps.meshPath;
			if (meshPath){
				mesh=Loader.getRes(innerResouMap[meshPath]);
				this.meshFilter.sharedMesh=mesh;
			};
			var materials=customProps.materials;
			if (materials){
				var sharedMaterials=render.sharedMaterials;
				var materialCount=materials.length;
				sharedMaterials.length=materialCount;
				for (var i=0;i < materialCount;i++)
				sharedMaterials[i]=Loader.getRes(innerResouMap[materials[i].path]);
				render.sharedMaterials=sharedMaterials;
			}
		}
	}

	/**
	*@private
	*/
	__proto._applyMeshMaterials=function(mesh){
		var shaderMaterials=this._render.sharedMaterials;
		var meshMaterials=mesh.materials;
		for (var i=0,n=meshMaterials.length;i < n;i++)
		(shaderMaterials[i])|| (shaderMaterials[i]=meshMaterials[i]);
		this._render.sharedMaterials=shaderMaterials;
	}

	/**
	*@inheritDoc
	*/
	__proto._addToInitStaticBatchManager=function(){
		MeshSprite3D._staticBatchManager._addInitBatchSprite(this);
	}

	/**
	*@inheritDoc
	*/
	__proto.cloneTo=function(destObject){
		var meshSprite3D=destObject;
		(meshSprite3D._geometryFilter).sharedMesh=(this._geometryFilter).sharedMesh;
		var meshRender=this._render;
		var destMeshRender=meshSprite3D._render;
		destMeshRender.enable=meshRender.enable;
		destMeshRender.sharedMaterials=meshRender.sharedMaterials;
		destMeshRender.castShadow=meshRender.castShadow;
		var lightmapScaleOffset=meshRender.lightmapScaleOffset;
		lightmapScaleOffset && (destMeshRender.lightmapScaleOffset=lightmapScaleOffset.clone());
		destMeshRender.lightmapIndex=meshRender.lightmapIndex;
		destMeshRender.sortingFudge=meshRender.sortingFudge;
		laya.d3.core.Sprite3D.prototype.cloneTo.call(this,destObject);
	}

	/**
	*@inheritDoc
	*/
	__proto.destroy=function(destroyChild){
		(destroyChild===void 0)&& (destroyChild=true);
		if (this.destroyed)
			return;
		var mesh=this.meshFilter.sharedMesh;
		(mesh.loaded)|| (mesh.off(/*laya.events.Event.LOADED*/"loaded",this,this._applyMeshMaterials));
		_super.prototype.destroy.call(this,destroyChild);
		(this._geometryFilter)._destroy();
	}

	/**
	*获取网格过滤器。
	*@return 网格过滤器。
	*/
	__getset(0,__proto,'meshFilter',function(){
		return this._geometryFilter;
	});

	/**
	*获取网格渲染器。
	*@return 网格渲染器。
	*/
	__getset(0,__proto,'meshRender',function(){
		return this._render;
	});

	MeshSprite3D.load=function(url){
		return Laya.loader.create(url,null,null,MeshSprite3D);
	}

	return MeshSprite3D;
})(RenderableSprite3D)


})(window,document,Laya);

if (typeof define === 'function' && define.amd){
	define('laya.core', ['require', "exports"], function(require, exports) {
        'use strict';
        Object.defineProperty(exports, '__esModule', { value: true });
        for (var i in Laya) {
			var o = Laya[i];
            o && o.__isclass && (exports[i] = o);
        }
    });
}