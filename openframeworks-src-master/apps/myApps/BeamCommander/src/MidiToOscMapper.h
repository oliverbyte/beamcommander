#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxOsc.h"
#include <unordered_map>
#include <functional>

// Simple MIDI -> OSC mapper driven by a JSON config in bin/data/midi_mapping.json
// Default supports Akai APC40 mkII master fader mapping to /laser/brightness
class MidiToOscMapper : public ofxMidiListener {
public:
    struct CCMap {
    int channel = 1;        // 1-based MIDI channel. Use 0 as a wildcard (match any channel).
        int cc = 14;            // control change number
        std::string osc = "/laser/brightness"; // OSC address
        float scale = 1.0f;     // multiply normalized value
        float offset = 0.0f;    // add after scaling
        bool invert = false;    // 1-v (absolute) or reverse direction (relative)
        bool relative = false;  // treat values as relative deltas (endless encoders)
        float step = 0.01f;     // per-tick step when relative=true (normalized 0..1)
    // Centered absolute mode: map MIDI 0..127 to -1..1 with exact 0 at center (63/64).
    // Useful for bidirectional controls like position X/Y. When enabled, scale/offset are ignored.
    bool centered = false;
    // Deadzone around center (0..1 of full range) where output snaps to 0 when |value| < deadzone.
    float deadzone = 0.0f;
        // Optional value range mapping from normalized 0..1 -> [min,max]
        float outMin = 0.0f;
        float outMax = 1.0f;
        // Optional curve. If gamma != 1, apply pow(normalized, gamma) before mapping to [outMin,outMax].
        float gamma = 1.0f;
    };

    struct NoteMap {
        int channel = 1;                // 1-based MIDI channel
        int note = 60;                  // MIDI note number
        std::string osc = "/flash";    // OSC address to send on press/release
        float onValue = 1.0f;           // value to send on NOTE_ON (or NOTE_ON velocity>0)
        float offValue = 0.0f;          // value to send on NOTE_OFF (or NOTE_ON velocity==0)
        bool velocityAsValue = false;   // if true, send velocity/127 on NOTE_ON instead of onValue
    // Optional: exclusive group name. Only the most recently pressed note in the group stays lit.
    std::string exclusiveGroup;     // empty => no exclusivity handling
    // LED feedback values to send back to device (APC40) via MIDI OUT. 0=off.
    int ledOn = 1;                  // velocity to send to keep pad lit (background)
    int ledOff = 0;                 // velocity to send to turn pad off
    bool ledFeedback = false;       // enable sending LED feedback for this note
    bool toggle = false;            // if true, NOTE_ON toggles between onValue/offValue; NOTE_OFF ignored
    };

    // If midiPortSubstring == "*" open ALL available MIDI input ports (first matching port still used for LED feedback output)
    bool setup(const std::string& midiPortSubstring = "APC", int oscPort = 9000){
    ofLogNotice() << "MidiToOscMapper: setup begin";
        loadConfig();
        // Debug: list all CC mappings
        for(const auto &m : ccMaps){
            std::string chStr = (m.channel==0?std::string("any"):ofToString(m.channel));
            ofLogNotice() << "CCMap: channel=" << chStr << " cc=" << m.cc << " osc=" << m.osc
                          << " outMin=" << m.outMin << " outMax=" << m.outMax;
        }
        // Verify there are no duplicate channel+cc pairs mapped to different OSC endpoints.
        std::map<std::pair<int,int>, std::string> uniquePairs;
        for(const auto &m : ccMaps){
            auto key = std::make_pair(m.channel, m.cc);
            auto it = uniquePairs.find(key);
            if(it == uniquePairs.end()){
                uniquePairs[key] = m.osc;
            }else if(it->second != m.osc){
                ofLogWarning() << "Duplicate MIDI CC mapping detected for channel " << m.channel
                               << " cc " << m.cc << " mapping to both '" << it->second
                               << "' and '" << m.osc << "'. Only first will effectively trigger; please resolve.";
            }
        }
        // MIDI
        midi.listInPorts();
        auto ports = midi.getInPortList();
        std::vector<int> openIndices;
        if(midiPortSubstring == "*"){
            // Open every available port
            for(size_t i=0;i<ports.size();++i){
                openIndices.push_back((int)i);
            }
        } else {
            int chosen = -1;
            for(size_t i=0;i<ports.size();++i){
                const std::string &name = ports[i];
                if(ofIsStringInString(ofToLower(name), ofToLower(midiPortSubstring))){
                    chosen = (int)i;
                    break;
                }
            }
            if(chosen < 0 && !ports.empty()){
                ofLogWarning() << midiPortSubstring << " device not found, opening first MIDI port: " << ports[0];
                chosen = 0;
            }
            if(chosen >= 0){
                openIndices.push_back(chosen);
            }
        }
        if(openIndices.empty()){
            ofLogWarning() << "No MIDI ports available";
        } else {
            bool first = true;
            for(int idx : openIndices){
                ofxMidiIn *in = nullptr;
                if(first){
                    // use primary 'midi'
                    if(!midi.openPort(idx)){
                        ofLogError() << "Failed to open MIDI port index " << idx;
                        continue;
                    }
                    midiPortName = ports[idx];
                    in = &midi;
                } else {
                    // create additional port
                    auto extra = std::make_unique<ofxMidiIn>();
                    if(!extra->openPort(idx)){
                        ofLogError() << "Failed to open additional MIDI port index " << idx;
                        continue;
                    }
                    in = extra.get();
                    extraMidiIns.push_back(std::move(extra));
                }
                if(in){
                    ofLogNotice() << "Opened MIDI IN port: (" << idx << ") '" << ports[idx] << "'";
                    in->addListener(this);
                    in->ignoreTypes(false, false, true);
                }
                if(first){
                    // Attempt to open OUT for LED feedback from the first successfully opened port
                    if(!midiOut.openPort(idx)){
                        ofLogError() << "Failed to open MIDI OUT port index " << idx;
                    } else {
                        ofLogNotice() << "Opened MIDI OUT port for LED feedback: index " << idx << " name '" << ports[idx] << "'";
                        for(const auto &nm : noteMaps){
                            if(nm.ledFeedback){
                                midiOut.sendNoteOn(nm.channel, nm.note, nm.ledOff);
                            }
                        }
                    }
                }
                first = false;
            }
        }

        // OSC
        sender.setup("127.0.0.1", oscPort);
        return true;
    }

    void exit(){
    midi.removeListener(this);
    midi.closePort();
    midiOut.closePort();
    for(auto &p : extraMidiIns){
        if(p){
            p->removeListener(this);
            p->closePort();
        }
    }
    }

    void newMidiMessage(ofxMidiMessage& msg) override {
        if(msg.status == MIDI_CONTROL_CHANGE){
            const int ch = (int)msg.channel; // 1..16
            const int cc = (int)msg.control;
            const int val = (int)msg.value; // 0..127
            float v = ofClamp(val / 127.0f, 0.0f, 1.0f);
            // Unconditional log for learn/mapping
            ofLogNotice() << "MIDI CC [" << (msg.portName.empty()?midiPortName:msg.portName) << "] ch "
                          << ch << " cc " << cc << " val " << val;
            // Check explicit CC maps first
            for(const auto& m : ccMaps){
                if((m.channel == 0 || m.channel == ch) && m.cc == cc){
                    float norm = 0.0f; // normalized 0..1 before range map (or centered value in -1..1 when centered=true)
                    if(m.relative){
                        // Two's complement style: 1..63 = +delta, 65..127 = negative
                        int delta = (val <= 63) ? val : (val - 128); // e.g., 127 -> -1, 126 -> -2
                        float d = delta * m.step;
                        if(m.invert) d = -d; // invert direction for relative encoders
                        uint32_t key = ((uint32_t)ch << 8) | (uint32_t)cc;
                        float acc = 0.0f;
                        auto it = ccAcc.find(key);
                        if(it != ccAcc.end()) acc = it->second;
                        acc = ofClamp(acc + d, 0.0f, 1.0f);
                        ccAcc[key] = acc;
                        norm = acc;
                    } else {
                        if(m.centered){
                            // Map 0..63 -> [-1..0], 64..127 -> [0..1], exact 0 at 63 or 64
                            float c = 0.0f;
                            if(val >= 64) c = (float)(val - 64) / 63.0f;       // 64 -> 0, 127 -> 1
                            else            c = (float)(val - 63) / 63.0f;       // 63 -> 0, 0   -> -1
                            if(m.invert) c = -c; // invert axis
                            // Symmetric gamma around center
                            c = ofClamp(c, -1.0f, 1.0f);
                            if(m.gamma > 0.0f && m.gamma != 1.0f){
                                float s = (c >= 0.0f) ? 1.0f : -1.0f;
                                c = s * powf(fabsf(c), m.gamma);
                            }
                            // Deadzone around center
                            if(m.deadzone > 0.0f && fabsf(c) < m.deadzone){
                                c = 0.0f;
                            }
                            // Map from [-1,1] to [outMin,outMax]
                            float out = ofMap(c, -1.0f, 1.0f, m.outMin, m.outMax, true);
                            ofxOscMessage om; om.setAddress(m.osc);
                            om.addFloatArg(out);
                            ofLogNotice() << "MIDI CC ch " << ch << " cc " << cc << " val " << val
                                          << " -> OSC " << m.osc << " " << out;
                            sender.sendMessage(om, false);
                            continue; // done for centered branch
                        } else {
                            norm = v;
                            if(m.invert) norm = 1.0f - norm;
                            norm = norm * m.scale + m.offset;
                            norm = ofClamp(norm, 0.0f, 1.0f);
                        }
                    }
                    if(m.gamma > 0.0f && m.gamma != 1.0f){
                        norm = powf(ofClamp(norm, 0.0f, 1.0f), m.gamma);
                    }
                    float out = ofMap(norm, 0.0f, 1.0f, m.outMin, m.outMax, true);
                    ofxOscMessage om; om.setAddress(m.osc);
                    om.addFloatArg(out);
                    ofLogNotice() << "MIDI CC ch " << ch << " cc " << cc << " val " << val
                                  << " -> OSC " << m.osc << " " << out;
                    sender.sendMessage(om, false);
                }
            }
        }
    else if(msg.status == MIDI_NOTE_ON || msg.status == MIDI_NOTE_OFF){
            const int ch = (int)msg.channel; // 1..16
            const int n  = (int)msg.pitch;   // note number
            const int vel = (int)msg.velocity; // 0..127
            const bool isOn = (msg.status == MIDI_NOTE_ON) && (vel > 0);
            const bool isOff = (msg.status == MIDI_NOTE_OFF) || ((msg.status == MIDI_NOTE_ON) && vel == 0);
            // Unconditional log for learn/mapping
            ofLogNotice() << "MIDI NOTE [" << (msg.portName.empty()?midiPortName:msg.portName) << "] ch "
                          << ch << " note " << n << (isOn?" ON":" OFF") << " vel " << vel;
            // Notify raw note callback early (only on NOTE_ON w/vel>0) for learn feature
            if(isOn && onRawNote){
                onRawNote(ch, n);
            }
            for(const auto& m : noteMaps){
                if(m.channel != ch || m.note != n) continue;
                if(m.toggle && isOn){
                    // Flip stored toggle state
                    uint32_t key = ((uint32_t)ch << 8) | (uint32_t)n;
                    bool cur = false;
                    auto it = toggleStates.find(key);
                    if(it != toggleStates.end()) cur = it->second;
                    bool next = !cur;
                    toggleStates[key] = next;
                    float out = next ? m.onValue : m.offValue;
                    ofxOscMessage om; om.setAddress(m.osc);
                    om.addFloatArg(out);
                    ofLogNotice() << "MIDI Note TOGGLE ch " << ch << " note " << n << " -> OSC " << m.osc << " " << out;
                    sender.sendMessage(om, false);
                    if(m.ledFeedback){
                        midiOut.sendNoteOn(m.channel, m.note, next ? m.ledOn : m.ledOff);
                    }
                    continue; // do not process further
                }
                if(m.toggle) {
                    // Ignore NOTE_OFF for toggle notes
                    if(isOff) continue;
                }
                // Exclusive (radio) behavior: on press, set this ON and all others in group OFF. Ignore release for exclusives.
                bool isExclusive = !m.exclusiveGroup.empty();
                if(isExclusive){
                    if(isOn){
                        // Send ON for this one first (prevents perceived flicker where LED momentarily goes dim)
                        float out = m.velocityAsValue ? ofClamp(vel / 127.0f, 0.0f, 1.0f) : m.onValue;
                        ofxOscMessage omOn; omOn.setAddress(m.osc);
                        omOn.addFloatArg(out);
                        ofLogNotice() << "MIDI Note ch " << ch << " note " << n << " ON -> OSC " << m.osc << " " << out;
                        sender.sendMessage(omOn, false);
                        if(m.ledFeedback){
                            // Update active mapping and light LED
                            activeExclusive[m.exclusiveGroup] = std::make_pair(m.channel, m.note);
                            midiOut.sendNoteOn(m.channel, m.note, m.ledOn);
                            // Extra safety: refresh entire group to ensure device shows dim colors for others & bright for selected
                            refreshExclusiveGroupLEDs(m.exclusiveGroup);
                        }
                        // Now turn OFF (dim) others in group (OSC + LED) so only one stays active
                        for(const auto &other : noteMaps){
                            if(&other == &m) continue;
                            if(other.channel == m.channel && other.exclusiveGroup == m.exclusiveGroup){
                                bool suppressOscOff = (m.exclusiveGroup == "shapes" || m.exclusiveGroup == "colors" || m.exclusiveGroup == "movement"); // suppress OFF messages for radio groups
                                if(!suppressOscOff){
                                    ofxOscMessage omOff; omOff.setAddress(other.osc);
                                    omOff.addFloatArg(other.offValue);
                                    ofLogNotice() << "MIDI Note ch " << ch << " note " << other.note << " FORCED OFF -> OSC " << other.osc << " " << other.offValue;
                                    sender.sendMessage(omOff, false);
                                } else {
                                    ofLogNotice() << "MIDI Note ch " << ch << " note " << other.note << " FORCED OFF (LED only, shapes group suppresses OSC OFF)";
                                }
                                if(other.ledFeedback){
                                    midiOut.sendNoteOn(other.channel, other.note, other.ledOff); // dim background
                                }
                            }
                        }
                    } else if(isOff){
                        // Ignore OSC off for exclusives, but some controllers (APC40) may clear LED on release;
                        // re-send LED state to keep it latched visually.
                        if(m.ledFeedback){
                            // Ensure active still recorded (should already be) then refresh
                            auto itSel = activeExclusive.find(m.exclusiveGroup);
                            if(itSel != activeExclusive.end() && itSel->second.first == m.channel && itSel->second.second == m.note){
                                midiOut.sendNoteOn(m.channel, m.note, m.ledOn);
                            }
                            refreshExclusiveGroupLEDs(m.exclusiveGroup);
                        }
                        ofLogNotice() << "MIDI Note ch " << ch << " note " << n << " OFF ignored (exclusive latch, LED reasserted)";
                    }
                } else {
                    // Non-exclusive: normal momentary behavior
                    if(!(isOn || isOff)) continue;
                    bool isCueTrigger = ofIsStringInString(m.osc, "/cue/");
                    bool isMomentaryCue = ofIsStringInString(m.osc, "/cue/momentary/");
                    if(isOff && isCueTrigger && !isMomentaryCue){
                        // Suppress OFF for cue triggers to avoid double loading
                        if(m.ledFeedback){
                            midiOut.sendNoteOn(m.channel, m.note, m.ledOff);
                        }
                        continue;
                    }
                    float out = isOn ? (m.velocityAsValue ? ofClamp(vel / 127.0f, 0.0f, 1.0f) : m.onValue) : m.offValue;
                    ofxOscMessage om; om.setAddress(m.osc);
                    om.addFloatArg(out);
                    ofLogNotice() << "MIDI Note ch " << ch << " note " << n << (isOn?" ON":" OFF") << " -> OSC " << m.osc << " " << out;
                    sender.sendMessage(om, false);
                    if(m.ledFeedback){
                        if(isOn){
                            midiOut.sendNoteOn(m.channel, m.note, m.ledOn);
                        } else if(isOff){
                            midiOut.sendNoteOn(m.channel, m.note, m.ledOff);
                        }
                    }
                }
            }
        }
    }

    // Ensure all notes in an exclusive group have correct LED states (selected bright, others dim)
    void refreshExclusiveGroupLEDs(const std::string &group){
        auto itSel = activeExclusive.find(group);
        for(const auto &nm : noteMaps){
            if(nm.exclusiveGroup != group || !nm.ledFeedback) continue;
            int vel = (itSel != activeExclusive.end() && itSel->second.first == nm.channel && itSel->second.second == nm.note) ? nm.ledOn : nm.ledOff;
            midiOut.sendNoteOn(nm.channel, nm.note, vel);
            ofLogNotice() << "LED refresh group='" << group << "' note=" << nm.note << " velocity=" << vel;
        }
    }

    // Load mapping from data/midi_mapping.json
    void loadConfig(){
        const std::string path = ofToDataPath("midi_mapping.json", true);
    configPath = path;
        if(!ofFile::doesFileExist(path)){
            // Default: APC40 master -> /laser/brightness 0..1
            ccMaps.clear();
            noteMaps.clear();
            CCMap def; def.channel = 1; def.cc = 14; def.osc = "/laser/brightness"; def.outMin = 0.0f; def.outMax = 1.0f;
            ccMaps.push_back(def);
            // Default scanrate: APC40 mkII fader on MIDI channel 8 CC7 -> /laser/scanrate (normalized 0..1 -> min..max PPS)
            CCMap scan; scan.channel = 8; /* only accept CC7 on channel 8 */ scan.cc = 7; scan.osc = "/laser/scanrate"; scan.outMin = 0.0f; scan.outMax = 1.0f; ccMaps.push_back(scan);
            // Default dotted effect amount: channel 7 CC7 -> /laser/dotted (0..1)
            CCMap dotted; dotted.channel = 7; dotted.cc = 7; dotted.osc = "/laser/dotted"; dotted.outMin = 0.0f; dotted.outMax = 1.0f; ccMaps.push_back(dotted);
            // Rainbow controls (channel 1):
            // CC51 -> rainbow speed (bi-directional). Use centered mapping to emit -1..1 which /laser/rainbow/speed expects.
            CCMap rbSpd; rbSpd.channel = 1; rbSpd.cc = 51; rbSpd.osc = "/laser/rainbow/speed"; rbSpd.centered = true; rbSpd.outMin = -1.0f; rbSpd.outMax = 1.0f; ccMaps.push_back(rbSpd);
            // CC52 -> rainbow size (amount 0..1 spatial spread)
            CCMap rbAmt; rbAmt.channel = 1; rbAmt.cc = 52; rbAmt.osc = "/laser/rainbow/amount"; rbAmt.outMin = 0.0f; rbAmt.outMax = 1.0f; ccMaps.push_back(rbAmt);
            // CC53 -> rainbow blend (0 hard bands .. 1 smooth)
            CCMap rbBlend; rbBlend.channel = 1; rbBlend.cc = 53; rbBlend.osc = "/laser/rainbow/blend"; rbBlend.outMin = 0.0f; rbBlend.outMax = 1.0f; ccMaps.push_back(rbBlend);
            // Default footswitch guesses: CC64 or Note64 -> /flash
            CCMap fscc; fscc.channel = 1; fscc.cc = 64; fscc.osc = "/flash"; fscc.outMin = 0.0f; fscc.outMax = 1.0f;
            ccMaps.push_back(fscc);
            NoteMap fsnote; fsnote.channel = 1; fsnote.note = 64; fsnote.osc = "/flash"; fsnote.onValue = 1.0f; fsnote.offValue = 0.0f; fsnote.velocityAsValue = false;
            noteMaps.push_back(fsnote);
            ofLogNotice() << "No midi_mapping.json found; using default APC40 master mapping";
            return;
        }
        try{
            ofJson j = ofLoadJson(path);
            ccMaps.clear();
            noteMaps.clear();
        if(j.contains("cc") && j["cc"].is_array()){
                for(const auto& e : j["cc"]) {
                    CCMap m;
                    m.channel = e.value("channel", 1);
                    m.cc = e.value("cc", 14);
                    m.osc = e.value("osc", std::string("/laser/brightness"));
                    m.scale = e.value("scale", 1.0f);
                    m.offset = e.value("offset", 0.0f);
                    m.invert = e.value("invert", false);
            m.relative = e.value("relative", false);
            m.step = e.value("step", 0.01f);
                    m.outMin = e.value("outMin", 0.0f);
                    m.outMax = e.value("outMax", 1.0f);
                    m.gamma = e.value("gamma", 1.0f);
                    m.centered = e.value("centered", false);
                    m.deadzone = e.value("deadzone", 0.0f);
                    ccMaps.push_back(m);
                }
            }
        if(j.contains("notes") && j["notes"].is_array()){
                for(const auto& e : j["notes"]) {
                    NoteMap m;
                    m.channel = e.value("channel", 1);
                    m.note = e.value("note", 60);
                    m.osc = e.value("osc", std::string("/flash"));
                    m.onValue = e.value("onValue", 1.0f);
                    m.offValue = e.value("offValue", 0.0f);
                    m.velocityAsValue = e.value("velocityAsValue", false);
                m.exclusiveGroup = e.value("exclusiveGroup", std::string(""));
                m.ledOn = e.value("ledOn", 1);
                m.ledOff = e.value("ledOff", 0);
                m.ledFeedback = e.value("ledFeedback", false);
                m.toggle = e.value("toggle", false);
                    noteMaps.push_back(m);
                }
            }
            ofLogNotice() << "Loaded " << ccMaps.size() << " CC mappings and " << noteMaps.size() << " note mappings from " << path;
            for(const auto &nm : noteMaps){
                ofLogNotice() << "NoteMap: ch=" << nm.channel << " note=" << nm.note << " osc=" << nm.osc
                              << (nm.toggle?" (toggle)":"")
                              << (nm.exclusiveGroup.empty()?"":" group="+nm.exclusiveGroup);
            }
            // Ensure rainbow CC mappings exist (add if missing). Auto-persist if we add any.
            auto hasCC = [&](int ch,int cc){
                for(const auto &m : ccMaps){ if(m.channel==ch && m.cc==cc) return true; } return false; };
            bool added=false;
            if(!hasCC(1,51)){
                CCMap m; m.channel=1; m.cc=51; m.osc="/laser/rainbow/speed"; m.centered=true; m.outMin=-1.0f; m.outMax=1.0f; ccMaps.push_back(m); added=true; ofLogNotice() << "Auto-added CC51 -> /laser/rainbow/speed"; }
            if(!hasCC(1,52)){
                CCMap m; m.channel=1; m.cc=52; m.osc="/laser/rainbow/amount"; m.outMin=0.0f; m.outMax=1.0f; ccMaps.push_back(m); added=true; ofLogNotice() << "Auto-added CC52 -> /laser/rainbow/amount"; }
            if(!hasCC(1,53)){
                CCMap m; m.channel=1; m.cc=53; m.osc="/laser/rainbow/blend"; m.outMin=0.0f; m.outMax=1.0f; ccMaps.push_back(m); added=true; ofLogNotice() << "Auto-added CC53 -> /laser/rainbow/blend"; }
            if(added){
                saveConfig(); // persist additions
            }
        }catch(...){
            ofLogError() << "Failed to parse midi_mapping.json";
        }
    }

    // Persist current mappings back to JSON file
    void saveConfig(){
        if(configPath.empty()) configPath = ofToDataPath("midi_mapping.json", true);
        ofJson root;
        auto &ccArr = root["cc"] = ofJson::array();
        for(const auto &m : ccMaps){
            ofJson e;
            e["channel"] = m.channel; e["cc"] = m.cc; e["osc"] = m.osc; e["scale"] = m.scale; e["offset"] = m.offset;
            e["invert"] = m.invert; e["relative"] = m.relative; e["step"] = m.step; e["outMin"] = m.outMin; e["outMax"] = m.outMax;
            e["gamma"] = m.gamma; e["centered"] = m.centered; e["deadzone"] = m.deadzone; ccArr.push_back(e);
        }
        auto &noteArr = root["notes"] = ofJson::array();
        for(const auto &m : noteMaps){
            ofJson e;
            e["channel"] = m.channel; e["note"] = m.note; e["osc"] = m.osc; e["onValue"] = m.onValue; e["offValue"] = m.offValue;
            e["velocityAsValue"] = m.velocityAsValue; e["exclusiveGroup"] = m.exclusiveGroup; e["ledOn"] = m.ledOn; e["ledOff"] = m.ledOff;
            e["ledFeedback"] = m.ledFeedback; e["toggle"] = m.toggle; noteArr.push_back(e);
        }
        ofSavePrettyJson(configPath, root);
        ofLogNotice() << "MidiToOscMapper: saved mappings to " << configPath;
    }

    // Add or update a note mapping to trigger a cue
    void addOrUpdateCueNoteMapping(int channel, int note, int cueIndex){
        if(cueIndex < 1) return;
    // Redirect legacy non-momentary API to momentary implementation to enforce new design.
    addOrUpdateMomentaryCueNoteMapping(channel, note, cueIndex);
    }

    // Add or update a note mapping to trigger a momentary cue (press apply, release restore)
    void addOrUpdateMomentaryCueNoteMapping(int channel, int note, int cueIndex){
        if(cueIndex < 1) return;
        std::string oscAddr = "/cue/momentary/" + ofToString(cueIndex);
        for(auto &nm : noteMaps){
            if(nm.channel == channel && nm.note == note){
                nm.osc = oscAddr;
                nm.onValue = 1.0f; nm.offValue = 0.0f; nm.toggle = false; nm.velocityAsValue = false;
                ofLogNotice() << "Updated existing MIDI note mapping ch " << channel << " note " << note << " -> " << oscAddr;
                saveConfig();
                return;
            }
        }
        NoteMap nm; nm.channel = channel; nm.note = note; nm.osc = oscAddr; nm.onValue = 1.0f; nm.offValue = 0.0f; nm.velocityAsValue = false;
        noteMaps.push_back(nm);
        ofLogNotice() << "Added new MIDI note mapping ch " << channel << " note " << note << " -> " << oscAddr;
        saveConfig();
    }
    // Dump current note mappings to log
    void dumpNoteMappings(){
        ofLogNotice() << "--- MIDI Note Mappings Dump (" << noteMaps.size() << ") ---";
        for(const auto &nm : noteMaps){
            ofLogNotice() << "ch=" << nm.channel << " note=" << nm.note << " osc=" << nm.osc;
        }
    }

    // Raw note callback (NOTE_ON only) used by learn logic in ofApp
    std::function<void(int,int)> onRawNote;

private:
    ofxMidiIn midi;
    ofxMidiOut midiOut; // MIDI OUT for LED feedback (APC40 pads)
    ofxOscSender sender;
    std::vector<CCMap> ccMaps;
    std::vector<NoteMap> noteMaps;
    std::string midiPortName;
    std::unordered_map<uint32_t, float> ccAcc; // accumulator for relative CCs (normalized 0..1)
    std::unordered_map<std::string, std::pair<int,int>> activeExclusive; // group -> (channel,note)
    std::unordered_map<uint32_t, bool> toggleStates; // key=(channel<<8)|note -> current toggle state
    std::string configPath; // last loaded/saved config
    std::vector<std::unique_ptr<ofxMidiIn>> extraMidiIns; // additional opened ports when using '*'
};
