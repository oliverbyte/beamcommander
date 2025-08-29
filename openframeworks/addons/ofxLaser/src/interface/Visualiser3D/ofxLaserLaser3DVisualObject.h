//
//  Object3D.h
//
//
//  Created by Seb Lee-Delisle on 10/06/2022.
//
#pragma once
#include "ofMain.h"
#include "ofxLaserObject3D.h"

namespace ofxLaser {

class Laser3DVisualObject : public Object3D{
    
    public :
    
    Laser3DVisualObject() {
        //params.setName("3D Visualisation");
        //visual3DParams.add(position);
        //visual3DParams.add(orientation);
        visual3DParams.add(horizontalRangeDegrees.set("Output Range horizontal", 60,0,180));
        visual3DParams.add(verticalRangeDegrees.set("Output Range vertical", 60,0,180));
        visual3DParams.add(flipX.set("Flip X", false));
        visual3DParams.add(flipY.set("Flip Y", false));
        
        ofAddListener(visual3DParams.parameterChangedE(), this, &Laser3DVisualObject::paramsChanged);
        ofAddListener(params.parameterChangedE(), this, &Laser3DVisualObject::paramsChanged);
    }
    ~Laser3DVisualObject() {
        ofRemoveListener(visual3DParams.parameterChangedE(), this, &Laser3DVisualObject::paramsChanged);
        ofRemoveListener(params.parameterChangedE(), this, &Laser3DVisualObject::paramsChanged);
    }
    
    void paramsChanged(ofAbstractParameter& e){
        //if(ignoreParamChange) return;
        //else
        glm::vec3 p = position.get();
        p.x = round(p.x);
        p.y = round(p.y);
        p.z = round(p.z);
        glm::vec3 o = orientation.get();
        
        o.x = round(p.x/5)*5;
        o.y = round(p.y/5)*5;
        o.z = round(p.z/5)*5;
        position.set(p);
       
        dirty = true;
    }

    ofParameter<int> horizontalRangeDegrees = 60;
    ofParameter<int> verticalRangeDegrees = 60;
    ofParameter<bool> flipX;
    ofParameter<bool> flipY;
    
    
    // Visualiser-specific parameters (output ranges and flips)
    ofParameterGroup visual3DParams;
    const ofParameterGroup& getVisual3DParams() const { return visual3DParams; }
    
    bool dirty = true; 
    // add serialization / deserialization
    
    // Ensure Visualiser 3D presets include flip/range params and detect changes
    void serialize(ofJson& json) const {
        // Serialize base Object3D params (position, orientation)
        Object3D::serialize(json);
        // Also persist visualiser-specific params so presets capture Flip X/Y and ranges
        ofSerialize(json["visual3DParams"], visual3DParams);
    }
    
    bool deserialize(ofJson& json){
        // Load base params first
        Object3D::deserialize(json);
        // Then load visualiser-specific params if present
        if(json.contains("visual3DParams")){
            ofDeserialize(json["visual3DParams"], visual3DParams);
        }
        return true;
    }
    
};
}
