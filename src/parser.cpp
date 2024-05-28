#include <parser.h>

CircuitParser::CircuitParser(circuit* ckt )
: ckt_(ckt) {};

// static variable definition
Macro* CircuitParser::topMacro_ = 0;
std::string CircuitParser::temp_groupName = "";
int CircuitParser::iter_i = -1;

// LEF parsing
int CircuitParser::LefSpacingCbk(
    lefrCallbackType_e c,
    lefiSpacing* l, 
    lefiUserData ud ) {
    circuit* ckt = (circuit*) ud;
    return 0;
}
int CircuitParser::LefPropCbk(
    lefrCallbackType_e c,
    lefiProp* p, 
    lefiUserData ud ) {
    circuit* ckt = (circuit*) ud;
    if(p->hasString()){
        std::string text = p->string();
        std::regex reg("EDGETYPE (\\d+) (\\d+) ([0-9.]+)"); // Regular expression matching three values after "EDGETYPE".
        std::sregex_iterator it(text.begin(), text.end(), reg); 
        std::sregex_iterator end;   
        while(it != end) {
            std::smatch match = *it;
            int type1 = std::stoi(match.str(1));
            int num2 = std::stoi(match.str(2));
            double num3 = std::stod(match.str(3));
            ckt->edge_table[type1+num2] = round(num3 / 0.1);
            ++it;
        }
    }
    return 0;
}
// SITE parsing
int CircuitParser::LefSiteCbk(
    lefrCallbackType_e c,
    lefiSite* si, 
    lefiUserData ud ) {

  circuit* ckt = (circuit*) ud;
  site* mySite = ckt->locateOrCreateSite( si->name() );
  if( si->hasSize() ) {
    mySite->width = si->sizeX();
    mySite->height = si->sizeY();
    ckt->min_width = mySite->width;
    ckt->defaultH = round(mySite->height / ckt->min_width);
    int temp_number = round(ckt->min_width / 0.1);
    for (int i = 0; i < 5; ++i)
    {
        ckt->edge_table[i] /= temp_number;
    }
  }

  if( si->hasClass() ) {
    mySite->type = si->siteClass();
  }

  if( si->hasXSymmetry() ) {
    mySite->symmetries.push_back("X");
  }
  if( si->hasYSymmetry() ) {
    mySite->symmetries.push_back("Y");
  }
  if( si->has90Symmetry() ) {
    mySite->symmetries.push_back("R90");
  }
  return 0;
}

// Layer Parsing
// No need to parse all data
int CircuitParser::LefLayerCbk(
    lefrCallbackType_e c,
    lefiLayer* la, 
    lefiUserData ud ) {
    circuit* ckt = (circuit*) ud;
    std::string::size_type position = std::string(la->name()).find("metal");
    if(position == std::string::npos) { 
        return 0;
    }
    layer* myLayer = ckt->locateOrCreateLayer( la->name() );
    if( la->hasType() ) { 
        myLayer->type = la->type(); 
        }
    if( la->hasDirection() ) { 
        myLayer->direction = la->direction(); 
        }

    if( la->hasPitch() ) { 
        myLayer->xPitch = myLayer->yPitch = la->pitch(); 
    }
    else if( la->hasXYPitch() ) { 
        myLayer->xPitch = la->pitchX(); 
        myLayer->yPitch = la->pitchY(); 
    }

    if( la->hasOffset() ) {
        myLayer->xOffset = myLayer->yOffset = la->offset(); 
    }
    else if( la->hasXYOffset() ) {
        myLayer->xOffset = la->offsetX();
        myLayer->yOffset = la->offsetY();
    }

    if( la->hasWidth() ) {
        myLayer->width = la->width(); 
    }
    return 0;
}

// MACRO parsing
// Begin Call back
// Set topMacro_ pointer
int CircuitParser::LefStartCbk(
    lefrCallbackType_e c,
    const char* name, 
    lefiUserData ud ) {
  circuit* ckt = (circuit*) ud;
  switch(c) {
    case lefrMacroBeginCbkType:
      // Fill topMacro_'s pointer
      topMacro_ = ckt->locateOrCreateMacro(name);
      break;
    default:
      break;
  }
  return 0;
}


int CircuitParser::LefMacroCbk(
    lefrCallbackType_e c,
    lefiMacro* ma, 
    lefiUserData ud ) {
  circuit* ckt = (circuit*) ud;

  if( ma->numProperties() > 0 ) {
    for(int i=0; i<ma->numProperties(); i++) {
      if( ma->propValue(i) ) {
          // cout << ma->propName(i) << " val: " << ma->propValue(i) << endl;
          std::string s(ma->propValue(i));
          std::regex rgx_left("LEFT (\\d+) ;");
          std::regex rgx_right("RIGHT (\\d+) ;");

          std::smatch match;
          int left_num, right_num;
          if (std::regex_search(s, match, rgx_left) && match.size() > 1) {
              std::istringstream(match.str(1)) >> left_num; // string to int
          }
          if (std::regex_search(s, match, rgx_right) && match.size() > 1) {
              std::istringstream(match.str(1)) >> right_num; // string to int
          }
          topMacro_->lEdgeT_ = left_num;
          topMacro_->rEdgeT_ = right_num;
      }
      else {
      //  cout << ma->propName(i) << " num: " << ma->propNum(i) << endl;
      }
    }
  }

  if( ma->hasClass() ) {
    topMacro_->type = ma->macroClass();
  }

  if( ma->hasOrigin() ) {
    topMacro_->xOrig = ma -> originX();
    topMacro_->yOrig = ma -> originY();
  }

  if( ma->hasSize() ) {
    topMacro_->width = ma->sizeX();
    topMacro_->height = ma->sizeY();
  }

  if( ma->hasSiteName() ) {
    site* mySite = ckt->locateOrCreateSite(ma->siteName());
    topMacro_->sites.push_back( ckt->site2id.find(mySite->name)->second );
  }

  
  return 0;
}

// Set macro's pin 
int CircuitParser::LefMacroPinCbk(
    lefrCallbackType_e c,
    lefiPin* pi, 
    lefiUserData ud ) {
  circuit* ckt = (circuit*) ud;

  Macro_pin myPin;

  std::string pinName = pi->name();
  myPin.name = pinName;
  if( pi->hasDirection() ) {
    myPin.direction = pi->direction();
  }
  if( pinName == "vdd"){
    int numVDD = pi->numPorts();
    topMacro_->numVDD = numVDD;
  }
  if( pinName == "vss"){
    int numVSS = pi->numPorts();
    topMacro_->numVSS = numVSS;
  }
//   if( pi->hasShape() ) {
//     myPin.shape = pi->shape(); 
//   }

  layer* curLayer = NULL;
  for(int i=0; i<pi->numPorts(); i++) {
    lefiGeometries* geom = pi->port(i);
    lefiGeomRect* lrect = NULL;
    lefiGeomPolygon* lpoly = NULL;
    double polyLx = DBL_MAX, polyLy = DBL_MAX;
    double polyUx = DBL_MIN, polyUy = DBL_MAX;


    for(int j=0; j<geom->numItems(); j++) {
      switch(geom->itemType(j)) {
        // when meets Layer .
        case lefiGeomLayerE:
          curLayer = ckt->locateOrCreateLayer( geom->getLayer(j) );
          break;
        
        // when meets Rect
        case lefiGeomRectE:
          lrect = geom->getRect(j);
          myPin.xLL = std::min(lrect->xl, myPin.xLL);
          myPin.yLL = std::min(lrect->yl, myPin.yLL);
          myPin.xUR = std::max(lrect->xh, myPin.xUR);
          myPin.yUR = std::max(lrect->yh, myPin.yUR);
        //   myPin.port.push_back(tmpRect);
          break;

        // when meets Polygon 
        case lefiGeomPolygonE:
          lpoly = geom->getPolygon(j);

          polyLx = DBL_MAX;
          polyLy = DBL_MAX;
          polyUx = DBL_MIN;
          polyUy = DBL_MIN;

          for(int k=0; k<lpoly->numPoints; k++) {
            polyLx = std::min(polyLx, lpoly->x[k]); 
            polyLy = std::min(polyLy, lpoly->y[k]);
            polyUx = std::max(polyUx, lpoly->x[k]);
            polyUy = std::max(polyUy, lpoly->y[k]);
          }
         
          myPin.xLL = polyLx;
          myPin.yLL = polyLy;
          myPin.xUR = polyUx;
          myPin.yUR = polyUy; 
        //   myPin.port.push_back(tmpRect);

        default:
          break;
      }
    }
  }
  myPin.layer = ckt->layer2id.find(curLayer->name)->second;
  myPin.xLL_offset_psite = int(myPin.xLL / ckt->min_width + 0.0000001);
  myPin.xUR_offset_psite = ceil(myPin.xUR / ckt->min_width - 0.0000001);
  double yLL_temp = myPin.yLL / ckt->min_width;
  double yUR_temp = myPin.yUR / ckt->min_width;
  myPin.yLL_offset_psite = int(yLL_temp / ckt->defaultH + 0.0000001) * ckt->defaultH;
  myPin.yUR_offset_psite = ceil(yUR_temp / ckt->defaultH - 0.0000001) * ckt->defaultH;
  topMacro_->pins[pinName] = myPin;
  if(pinName == ckt->FFClkPortName)
      topMacro_->isFlop = true;
  return 0; 
}

// Set macro's Obs
int CircuitParser::LefMacroObsCbk(
    lefrCallbackType_e c,
    lefiObstruction* obs,
    lefiUserData ud ) {
  circuit* ckt = (circuit*) ud;
  lefiGeometries* geom = obs->geometries();

  bool isMeetMetalLayer1 = false;
  for(int i=0; i<geom->numItems(); i++) {
    lefiGeomRect* lrect = NULL;
    Rect<double> tmpRect;

    switch(geom->itemType(i)) {
      // when meets metal1 segments.
      case lefiGeomLayerE:
        // HARD CODE
        // Need to be replaced layer. (metal1 name)
        isMeetMetalLayer1 = 
          (strcmp(geom->getLayer(i), "metal1") == 0)? true : false;
      break;
      // only metal1's obs should be pushed.
      case lefiGeomRectE:
        if(!isMeetMetalLayer1){ 
          break;
        }

        lrect = geom->getRect(i);
        tmpRect.xLL_ = lrect->xl;
        tmpRect.yLL_ = lrect->yl;
        tmpRect.xUR_ = lrect->xh;
        tmpRect.yUR_ = lrect->yh;
  
        topMacro_->obses.push_back(tmpRect);
        break;
      default: 
        break;    
    }
  }

//  cout << "obs: " << topMacro_->obses.size() << endl;
  return 0;
}

int CircuitParser::LefEndCbk(
    lefrCallbackType_e c,
    const char* name, 
    lefiUserData ud ) {
  circuit* ckt = (circuit*) ud;
  switch(c) {
    case lefrMacroEndCbkType:
//      cout << "Macro: " << topMacro_->name << " is undergoing test" << endl;
      ckt->read_lef_macro_define_top_power(topMacro_);
      // reset topMacro_'s pointer
      topMacro_ = 0;
      break;
    default:
      break;
  }
  return 0;
}



// - - - - - - - define multi row cell & define top power - - - - - - - - //
void circuit::read_lef_macro_define_top_power(Macro* myMacro) {

  bool isVddFound = false, isVssFound = false;
  std::string vdd_str, vss_str;

  auto pinPtr = myMacro->pins.find("vdd");
  if(pinPtr != myMacro->pins.end()) {
    vdd_str = "vdd";
    isVddFound = true;
  }
  else if( pinPtr != myMacro->pins.find("VDD") ) {
    vdd_str = "VDD";
    isVddFound = true;
  }

  pinPtr = myMacro->pins.find("vss");
  if( pinPtr != myMacro->pins.end()) {
    vss_str = "vss";
    isVssFound = true;
  }
  else if( pinPtr != myMacro->pins.find("VSS") ) {
    vss_str = "VSS";
    isVssFound = true;
  }

//   if( isVddFound || isVssFound ) {
    if(myMacro->numVDD == myMacro->numVSS) {
        myMacro->aligendRow = 2;//ANY odd-row-height
    } else if(myMacro->numVDD > myMacro->numVSS) {
        myMacro->aligendRow = 1; //VDD
    } else myMacro->aligendRow = 0; //VSS
//   }
}



int circuit::ReadLef(const std::vector<std::string>& lefStor) 
{
    FILE* f;
    fout = stdout;

    std::string tech_str = lefStor[0];
    std::string cell_str = lefStor[1];

    CircuitParser cp(this);
    void* userData = cp.Circuit();

    // sets the parser to be case sensitive...
    // default was supposed to be the case but false...
    // lefrSetCaseSensitivity(true);
    lefrInitSession(0);
    
    lefrSetUserData(cp.Circuit());
    // edge_table
    // lefrSetSpacingCbk(cp.LefSpacingCbk);
    lefrSetPropCbk(cp.LefPropCbk);
    // Site
    lefrSetSiteCbk(cp.LefSiteCbk);
    // Layer
    lefrSetLayerCbk(cp.LefLayerCbk);
    lefrSetMacroBeginCbk(cp.LefStartCbk);
    lefrSetMacroCbk(cp.LefMacroCbk);
    // lefrSetMacroCbk(macroCB);
    lefrSetPinCbk(cp.LefMacroPinCbk);
    lefrSetObstructionCbk(cp.LefMacroObsCbk);
    lefrSetMacroEndCbk(cp.LefEndCbk);

    lefrSetWarningLogFunction(printWarning);
    
    lefrSetLineNumberFunction(lineNumberCB);
    lefrSetDeltaNumberLines(10000);

    lefrSetRegisterUnusedCallbacks();
    lefrSetLogFunction(errorCB);
    lefrSetWarningLogFunction(warningCB);

    lefrSetAntennaInoutWarnings(30);
    lefrSetAntennaInputWarnings(30);
    lefrSetAntennaOutputWarnings(30);
    lefrSetArrayWarnings(30);
    lefrSetCaseSensitiveWarnings(30);
    lefrSetCorrectionTableWarnings(30);
    lefrSetDielectricWarnings(30);
    lefrSetEdgeRateThreshold1Warnings(30);
    lefrSetEdgeRateThreshold2Warnings(30);
    lefrSetEdgeRateScaleFactorWarnings(30);
    lefrSetInoutAntennaWarnings(30);
    lefrSetInputAntennaWarnings(30);
    lefrSetIRDropWarnings(30);
    lefrSetLayerWarnings(30);
    lefrSetMacroWarnings(30);
    lefrSetMaxStackViaWarnings(30);
    lefrSetMinFeatureWarnings(30);
    lefrSetNoiseMarginWarnings(30);
    lefrSetNoiseTableWarnings(30);
    lefrSetNonDefaultWarnings(30);
    lefrSetNoWireExtensionWarnings(30);
    lefrSetOutputAntennaWarnings(30);
    lefrSetPinWarnings(30);
    lefrSetSiteWarnings(30);
    lefrSetSpacingWarnings(30);
    lefrSetTimingWarnings(30);
    lefrSetUnitsWarnings(30);
    lefrSetUseMinSpacingWarnings(30);
    lefrSetViaRuleWarnings(30);
    // lefrSetViaWarnings(30);
    lefrSetViaWarnings(0);
    // is set to off or not set
    lefrSetOpenLogFileAppend();
    for(auto curLefLoc : lefStor) {
        lefrReset();
        if ((f = fopen(curLefLoc.c_str(),"r")) == 0) {
        std::cout << "Couldn't open input file " << curLefLoc << std::endl;
        return(2);
        }

        (void)lefrEnableReadEncrypted();

        int status = lefwInit(fout); // initialize the lef writer,
        // need to be called 1st
        if (status != LEFW_OK)
            return 1;

        int res = lefrRead(f, curLefLoc.c_str(), (void*)userData);

        if (res) {
        std::cout << "Reader returns bad status: " << curLefLoc << std::endl;
        exit(1);
        }
        else {
        std::cout << "Reading " << curLefLoc << " is Done" << std::endl;
        }


        (void)lefrReleaseNResetMemory();
    }
    (void)lefrUnsetCallbacks();
    void lefrUnsetMacroBeginCbk();
    void lefrUnsetMacroCbk();
    void lefrUnsetMacroEndCbk();
    void lefrUnsetLayerCbk();
    void lefrUnsetPinCbk();
    void lefrUnsetObstructionCbk();

    fclose(f);
    // Release allocated singleton data.
    lefrClear();    
    // printMacro(userData, string("ao12f01"));

    
    return 0;
}

// def parser cbk
int CircuitParser::DefDesignCbk(
    defrCallbackType_e c, 
    const char* string, 
    defiUserData ud) { 
  circuit* ckt = (circuit*) ud;

  ckt->design_name = string;
  return 0;
}
int CircuitParser::DefUnitsCbk(
    defrCallbackType_e c, 
    double d, 
    defiUserData ud) {
  circuit* ckt = (circuit*) ud;

  ckt->DEFdist2Microns = d;
  return 0;
}

int CircuitParser::DefDieAreaCbk(
    defrCallbackType_e c, 
    defiBox* box, 
    defiUserData ud ) {
  circuit* ckt = (circuit*) ud;

  ckt->lx = box->xl();
  ckt->by = box->yl();
  ckt->rx = box->xh()/static_cast<double>(ckt->DEFdist2Microns);
  ckt->ty = box->yh()/static_cast<double>(ckt->DEFdist2Microns);
  return 0;
}

// DEF's ROW parsing
int CircuitParser::DefRowCbk(
    defrCallbackType_e c, 
    defiRow* ro,
    defiUserData ud) {

  circuit* ckt = (circuit*) ud;
  Row* myRow = ckt->locateOrCreateRow( ro->name() );

  myRow->site = ckt->site2id.at( ro->macro() );
  myRow->origX = ro->x();
  myRow->origY = ro->y();
  myRow->siteorient = ro->orientStr();



  if( ro->hasDo() ){
    myRow->numSites = ro->xNum();
  }

  if( ro->hasDoStep() ) {
    myRow->stepX = ro->xStep();
    myRow->stepY = ro->yStep();
  }

  // initialize rowHeight variable (double)
  if( fabs(ckt->rowHeight - 0.0f) <= DBL_EPSILON ) {
    ckt->rowHeight = ckt->sites[ myRow->site ].height * ckt->DEFdist2Microns;
  }

  return 0;
}

// DEF's Start callback to reserve memories
int CircuitParser::DefStartCbk(
    defrCallbackType_e c,
    int num,
    defiUserData ud) {
  circuit* ckt = (circuit*) ud;
  switch(c) {
    case defrComponentStartCbkType:
      ckt->cells.reserve(num);
      ckt->defaultCellIds.reserve(num);
      ckt->cell_num = num;
      ckt->cellIds.reserve(num);
      break;
    case defrStartPinsCbkType:
      ckt->pins.reserve(num);
      break;
    case defrNetStartCbkType:
      ckt->nets.reserve(num);
      break;
    case defrGroupsStartCbkType:
      ckt->fenceRegions.resize(num);
      break;
    // case defrSNetStartCbkType:
    //   ckt->minVddCoordiY = DBL_MAX;
    //   break;
  }
  return 0; 
}

// DEF's COMPONENT parsing
int CircuitParser::DefComponentCbk(
    defrCallbackType_e c,
    defiComponent* co, 
    defiUserData ud) {

    circuit* ckt = (circuit*) ud;
    Cell* myCell = NULL;

  // newly inserted cells
    myCell = ckt->locateOrCreateCell( co->id() );
    myCell->type_ = ckt->macro2id[ co->name() ];

    myCell->name_ = co->id();
    myCell->ripup_cnt_ = 0;
    myCell->id_ = ckt->countComponents;
    ckt->cellIds.push_back(ckt->countComponents);
    /////////for fenceRegion cells
    std::string::size_type position = std::string(co->id()).find("/");
    if(position != std::string::npos) {  //exsit
      std::string groupName = std::string(co->id()).substr(0, position);
      ckt->group2cellIds[groupName].push_back(myCell->id_);
    }
    else {
        ckt->defaultCellIds.push_back(myCell->id_);
    }
    myCell->regionId_ = -1;
    Macro* myMacro  = &ckt->macros[ myCell->type_ ];
    if(myMacro->pins["vss"].yLL < 0) {
        myCell->isBottomVss_ = true;   //
        }
    else {
        myCell->isBottomVss_ = false;
    }
    if(ckt->doTech) {
      for(auto &pin : myMacro->pins) {
        if(pin.first != "vss" and pin.first != "vdd") {
          myCell->signal_pins_.emplace_back(pin.second.layer, pin.second.xLL_offset_psite, pin.second.xUR_offset_psite,
                    pin.second.yLL_offset_psite, pin.second.yUR_offset_psite);
        }
      }
    }
    myCell->aligendRow_ = myMacro->aligendRow;
    myCell->width_  = round(myMacro->width/ckt->min_width);  
    myCell->height_ = round(myMacro->height/ckt->min_width); 
    myCell->lEdgeT_ = myMacro->lEdgeT_;
    myCell->rEdgeT_ = myMacro->rEdgeT_;
    myCell->init_x_ = co->placementX()/static_cast<double>(ckt->DEFdist2Microns)/ckt->min_width;
    myCell->cur_x_ = static_cast<int>(myCell->init_x_ + 0.5);  //round
    myCell->cur_x_temp_ = myCell->cur_x_;
    myCell->init_y_ = co->placementY()/static_cast<double>(ckt->DEFdist2Microns)/ckt->min_width;
    myCell->cur_y_temp_ = myCell->init_y_;
    myCell->cellorient_ = co->placementOrientStr(); 
    if(co->isFixed()){
        myCell->isFixed_ = true;
        myCell->cur_y_ = int(myCell->init_y_  / ckt->defaultH + 0.0000001) * ckt->defaultH;
        int height_modified, width_modified;
        height_modified = myCell->height_;
        width_modified = myCell->width_;
        if(fabs(myCell->cur_y_- myCell->init_y_) > EPSILON) {
           height_modified+=ckt->defaultH;
        }
        myCell->cur_x_ = (int)(myCell->init_x_ + 0.0000001);  
        if(fabs(myCell->cur_x_- myCell->init_x_) > EPSILON) {
          ++width_modified;
        }
        Rect<int> rect(myCell->cur_x_, myCell->cur_y_, myCell->cur_x_+width_modified, myCell->cur_y_+height_modified);
        ckt->blockRegions.push_back(rect);
    }
    else {
        if(ckt->kMacros.find(myCell->height_) == ckt->kMacros.end()) {
            ckt->kMacros.emplace(myCell->height_, std::make_pair(1, 0.0));  //{height;nums,disp}
        }
        else{
            ++ckt->kMacros[myCell->height_].first;
        }       
        myCell->isFixed_ = false;
        ++ckt->num_unfixed_inst_;
    }

    ckt->countComponents++;
  return 0;
}

// Fence region handling
// DEF's REGIONS -> call groups
int CircuitParser::DefRegionCbk(
    defrCallbackType_e c,
    defiRegion* re, 
    defiUserData ud) {

    circuit* ckt = (circuit*) ud;
    FenceRegion *theRegion = ckt->locateOrCreateRegion(re->name());
    for(int i = 0; i < re->numRectangles(); i++) {
        double init_x = re->xl(i)/static_cast<double>(ckt->DEFdist2Microns)/ckt->min_width;
        double init_y = re->yl(i)/static_cast<double>(ckt->DEFdist2Microns)/ckt->min_width;
        int cur_y = int(init_y  / ckt->defaultH + 0.0000001) * ckt->defaultH;    
        int cur_x = (int)(init_x + 0.0000001);  

        int xUR = ceil(re->xh(i)/static_cast<double>(ckt->DEFdist2Microns)/ckt->min_width - 0.0000001);
        double yUR_temp = re->yh(i)/static_cast<double>(ckt->DEFdist2Microns)/ckt->min_width;
        int yUR = ceil(yUR_temp  / ckt->defaultH - 0.0000001) * ckt->defaultH; 
        cur_x = std::max(0, cur_x); //assert lx == 0;
        cur_y = std::max(0, cur_y);
        xUR = std::min(xUR, (int)round(ckt->rx/ckt->min_width));
        yUR = std::min(yUR, (int)round(ckt->ty/ckt->min_width));
        Rect<int> rect(cur_x, cur_y, xUR, yUR);
        theRegion->rects_.push_back(rect);
    }

  return 0;
}


int CircuitParser::DefPinCbk(
    defrCallbackType_e c, 
    defiPin* pi,
    defiUserData ud) {
  
    circuit* ckt = (circuit*) ud;
    Pin* myPin = nullptr;
    layer* myLayer = nullptr;
    myPin = ckt->locateOrCreatePin( pi->pinName(),"" );
    if( strcmp(pi->direction(), "INPUT") == 0 ) {
        myPin -> type = PI_PIN;
    }
    else if( strcmp(pi->direction(), "OUTPUT") == 0 ) {
        myPin -> type = PO_PIN;
    }

    myPin->isFixed = pi->isFixed();
    myPin->x_coord = pi->placementX();
    myPin->y_coord = pi->placementY();
    myLayer = ckt->locateOrCreateLayer(pi->layer(0));
    myPin->layer   = ckt->layer2id.find(myLayer->name)->second;
    int xl, yl, xh, yh;
    pi->bounds(0, &xl, &yl, &xh, &yh);
    double xLL_temp = xl;
    double xUR_temp = xh;
    double yLL_temp = yl;
    double yUR_temp = yh;
    if(ckt->doTech) {  // IO PIN 
      if(myPin->layer <= 2) {// metal3 and bottom metal
        SpNetRegion* spNetRegion = new SpNetRegion();
        double init_x = myPin->x_coord / static_cast<double>(ckt->DEFdist2Microns) / ckt->min_width;
        double init_y = myPin->y_coord / static_cast<double>(ckt->DEFdist2Microns) / ckt->min_width;
        double xLL_Offset = xLL_temp / static_cast<double>(ckt->DEFdist2Microns) / ckt->min_width;
        double xUR_Offset = xUR_temp / static_cast<double>(ckt->DEFdist2Microns) / ckt->min_width;
        double yLL_Offset = yLL_temp / static_cast<double>(ckt->DEFdist2Microns) / ckt->min_width;
        double yUR_Offset = yUR_temp / static_cast<double>(ckt->DEFdist2Microns) / ckt->min_width;
        int cur_y = int((init_y + yLL_Offset) / ckt->defaultH + 0.0000001) * ckt->defaultH;    
        int cur_x = (int)(init_x + xLL_Offset + 0.0000001);        

        int xUR = ceil(init_x + xUR_Offset - 0.0000001);
        int yUR = ceil((init_y + yUR_Offset)  / ckt->defaultH - 0.0000001) * ckt->defaultH;      

        cur_x = std::max(0, cur_x); //assert lx == 0;
        cur_y = std::max(0, cur_y);
        xUR = std::min(xUR, (int)round(ckt->rx/ckt->min_width));
        yUR = std::min(yUR, (int)round(ckt->ty/ckt->min_width));
        spNetRegion->layer_ = myPin->layer;
        spNetRegion->setRect(cur_x, cur_y, xUR, yUR);
        ckt->spNetRegions.push_back(spNetRegion);
      }
    }
    myPin->x_coord += 0.5 * (xLL_temp + xUR_temp); //centrial_x
    myPin->y_coord += 0.5 * (yLL_temp + yUR_temp); //centrial_y
    ckt->countPins++;
    return 0;
}


// DEF's SPECIALNETS
// Extract VDD/VSS row informations for mixed-height legalization
int CircuitParser::DefSNetWireCbk(
    defrCallbackType_e c,
    defiNet* swire, 
    defiUserData ud) {
    circuit* ckt = (circuit*) ud;
    return 0;
}
int CircuitParser::DefSNetCbk(
    defrCallbackType_e c,
    defiNet* swire, 
    defiUserData ud) {
    circuit* ckt = (circuit*) ud;
    double min_width = ckt->min_width;
    int  defaultH = ckt->defaultH, rx = ckt->rx, ty = ckt->ty;
    unsigned  DEFdist2Microns = ckt->DEFdist2Microns;
    int num_vss = 0;
    if( strcmp("vss", swire->name()) == 0 ||
        strcmp("VSS", swire->name()) == 0 ||
        strcmp("vdd", swire->name()) == 0 ||
        strcmp("VDD", swire->name()) == 0) {
        if( swire->numWires() ) {
            for(int i=0; i<swire->numWires(); i++) {
                defiWire* wire = swire->wire(i);
                for(int j=0; j<wire->numPaths(); j++){
                    defiPath* p = wire->path(j);
                    p->initTraverse();
                    double wOrh;
                    int layer;
                    int x1, y1, x3, y3;
                    int path = 0;
                    bool find_metal2_or_metal3 = false;
                    int pointCnt = 0;
                    while((path = (int)p->next()) != DEFIPATH_DONE ) {
                        switch(path) {
                            case DEFIPATH_LAYER:{
                                    if( strcmp( p->getLayer(), "metal2") == 0 or strcmp( p->getLayer(), "metal3") == 0 ) {
                                        find_metal2_or_metal3 = true;
                                        layer = ckt->layer2id.find(p->getLayer())->second;
                                    }
                                }
                                break;
                            case DEFIPATH_WIDTH:
                                if(find_metal2_or_metal3){
                                    int width = p->getWidth();
                                    wOrh = width / static_cast<double>(DEFdist2Microns) / min_width;
                                }
                                break;
                            case DEFIPATH_POINT:
                                if(find_metal2_or_metal3){
                                    if(pointCnt == 0) {
                                        p->getPoint(&x1, &y1);
                                        pointCnt++;
                                    }
                                    else if(pointCnt == 1){
                                        p->getPoint(&x3, &y3);
                                    }
                                }
                                break;
                            case DEFIPATH_VIA:
                                if(find_metal2_or_metal3){
                                    find_metal2_or_metal3 = false;
                                }
                                break;
                            default:
                                break;
                        }
                    }
                    if(find_metal2_or_metal3){
                        SpNetRegion* spNetRegion = new SpNetRegion();
                        spNetRegion->layer_ = layer;
                        double xLL_temp = x1 / static_cast<double>(DEFdist2Microns) / min_width; 
                        if(layer == 1){//metal2
                            double yLL_temp = y1 / static_cast<double>(DEFdist2Microns) / min_width;
                            int xLL = int(xLL_temp - 0.5 * wOrh + 0.0000001);
                            int yLL = int(yLL_temp / defaultH + 0.0000001) * defaultH;
                            int xUR = ceil(xLL_temp + 0.5 * wOrh - 0.0000001);
                            double yUR_temp = y3 / static_cast<double>(DEFdist2Microns) / min_width;
                            int yUR = ceil(yUR_temp  / defaultH - 0.0000001) * defaultH;
                            xUR = std::min(xUR, (int)round(ckt->rx/ckt->min_width));
                            yUR = std::min(yUR, (int)round(ckt->ty/ckt->min_width));
                            spNetRegion->setRect(xLL, yLL, xUR, yUR);
                            ckt->spNetRegions.push_back(spNetRegion);
                            ++num_vss;
                        }
                        else if(layer == 2){//metal3
                            double yLL_temp = y1 / static_cast<double>(DEFdist2Microns) / min_width;
                            int xLL = int(xLL_temp + 0.0000001);
                            int yLL = int((yLL_temp - 0.5 * wOrh) / defaultH + 0.0000001) * defaultH;
                            int yUR = ceil((yLL_temp + 0.5 * wOrh) / defaultH - 0.0000001) * defaultH;
                            double xUR_temp = x3/ static_cast<double>(DEFdist2Microns) / min_width;
                            int xUR = ceil(xUR_temp - 0.0000001);
                            xUR = std::min(xUR, (int)round(ckt->rx/ckt->min_width));
                            yUR = std::min(yUR, (int)round(ckt->ty/ckt->min_width));
                            spNetRegion->setRect(xLL, yLL, xUR, yUR);
                            ckt->spNetRegions.push_back(spNetRegion); 
                            ++num_vss;
                        }
                    }
                }
            }
        }
        if( swire->numRectangles()){
            for(int i=0; i<swire->numRectangles(); i++){
                std::string layerName = swire->rectName(i);
                int layer = ckt->layer2id.find(layerName)->second;
                int x1, y1, x3, y3;
                x1 = swire->xl(i),y1 = swire->yl(i),x3 = swire->xh(i),y3 = swire->yh(i);
                SpNetRegion* spNetRegion = new SpNetRegion();
                spNetRegion->layer_ = layer;
                double init_x = x1/static_cast<double>(DEFdist2Microns)/min_width;
                double init_y = y1/static_cast<double>(DEFdist2Microns)/min_width;
                int cur_y = int(init_y  / defaultH + 0.0000001) * defaultH;    
                int cur_x = (int)(init_x + 0.0000001);
                int xUR = ceil(x3/static_cast<double>(DEFdist2Microns)/min_width - 0.0000001);
                double yUR_temp = y3/static_cast<double>(DEFdist2Microns)/min_width;
                int yUR = ceil(yUR_temp  / defaultH - 0.0000001) * defaultH;  

                cur_x = std::max(0, cur_x); //assert lx == 0;
                cur_y = std::max(0, cur_y);
                xUR = std::min(xUR, (int)round(rx/min_width));
                yUR = std::min(yUR, (int)round(ty/min_width));
                spNetRegion->setRect(cur_x, cur_y, xUR, yUR);
                ckt->spNetRegions.push_back(spNetRegion);   
            }
        }
    }
  return 0;
}

// DEF's NET
int CircuitParser::DefNetCbk(
    defrCallbackType_e c,
    defiNet* dnet, 
    defiUserData ud) {
  circuit* ckt = (circuit*) ud;
  Net* myNet = NULL;

    myNet = ckt->locateOrCreateNet( dnet->name() );
    unsigned myNetId = ckt->net2id.find(myNet->name)->second;
    
    // subNet iterations
    for(int i=0; i<dnet->numConnections(); i++) {
        std::string instName;
        std::string portName;
        if(strcmp(dnet->instance(i), "PIN") == 0)
        {
            instName = dnet->pin(i);
            portName = "";
        }
        else
        {
            instName = dnet->instance(i);
            portName = dnet->pin(i);
        }
        Pin* myPin = ckt->locateOrCreatePin(instName, portName);
        myPin->net = myNetId; 
        if(i == 0) {
            myNet->source = myPin->id;
        }
        else {
            myNet->sinks.push_back(myPin->id);
        }
    }
    return 0;
}



// DEF's GROUPS -> call groups
int CircuitParser::DefGroupCbk(
    defrCallbackType_e c, 
    defiGroup* go,
    defiUserData ud) {
    circuit* ckt = (circuit*) ud;
    std::string groupName = go->name();
    std::cout << "Group " << groupName << " is found" << std::endl;
  return 0;
}
// DEF's GROUPS -> call groups
int CircuitParser::DefGroupNameCbk(
    defrCallbackType_e c, 
    const char* name,
    defiUserData ud) {
  circuit* ckt = (circuit*) ud;
  temp_groupName = name;
  iter_i++;
  return 0;
}

int CircuitParser::DefGroupMemberCbk(
    defrCallbackType_e c, 
    const char* name,
    defiUserData ud) {
    circuit* ckt = (circuit*) ud;
    std::string groupName = temp_groupName;
    std::string::size_type position = std::string(name).find("/");
    std::string compGroupName = std::string(name).substr(0, position);
    for(auto& cellId : ckt->group2cellIds[compGroupName]) {
        ckt->cells[cellId]->regionId_ = ckt->region2id[groupName];
        // cells_cp[cellId]->regionId_ = ckt->region2id[groupName];
    }
    ckt->region2cellIds[groupName].insert(ckt->region2cellIds[groupName].end(), ckt->group2cellIds[compGroupName].begin(), ckt->group2cellIds[compGroupName].end());
    if (iter_i >= ckt->fenceCellIds.size()) {
        ckt->fenceCellIds.resize(iter_i + 1);
    }
    ckt->fenceCellIds[iter_i].insert(ckt->fenceCellIds[iter_i].end(), ckt->group2cellIds[compGroupName].begin(), ckt->group2cellIds[compGroupName].end());
    ckt->group2cellIds[compGroupName].resize(0);

    return 0;
}

// DEF's BLOCKAGES
int CircuitParser::DefBlockageCbk(
    defrCallbackType_e c, 
    defiBlockage* block, 
    defiUserData ud) {
    circuit* ckt = (circuit*) ud;
    if(block->hasPlacement()){
        double init_x = block->xl(0)/static_cast<double>(ckt->DEFdist2Microns)/ckt->min_width;
        double init_y = block->yl(0)/static_cast<double>(ckt->DEFdist2Microns)/ckt->min_width;

        int cur_y = int(init_y  / ckt->defaultH + 0.0000001) * ckt->defaultH;
        int cur_x = (int)(init_x + 0.0000001);  //round

        int xUR = ceil(block->xh(0)/static_cast<double>(ckt->DEFdist2Microns)/ckt->min_width - 0.0000001);
        double yUR_temp = block->yh(0)/static_cast<double>(ckt->DEFdist2Microns)/ckt->min_width;
        int yUR = ceil(yUR_temp  / ckt->defaultH - 0.0000001) * ckt->defaultH;

        cur_x = std::max(0, cur_x); //assert lx == 0;
        cur_y = std::max(0, cur_y);
        xUR = std::min(xUR, (int)round(ckt->rx/ckt->min_width));
        yUR = std::min(yUR, (int)round(ckt->ty/ckt->min_width));
        Rect<int> rect(cur_x, cur_y, xUR, yUR);
        ckt->blockRegions.push_back(rect);
    }
    return 0;
}


// DEF's End callback
int CircuitParser::DefEndCbk(
    defrCallbackType_e c,
    void*,
    defiUserData ud) {
    circuit* ckt = (circuit*) ud;
    switch(c) {
        case defrDesignEndCbkType:
            break;
        case defrGroupsEndCbkType:
            for(auto it = ckt->group2cellIds.begin(); it != ckt->group2cellIds.end(); ++it) {
                if(!it->second.empty()) {
                    ckt->defaultCellIds.insert(ckt->defaultCellIds.end(), it->second.begin(), it->second.end());
                // cout << "Group " << it->first << " is not used" << endl;
                }
            }  
            // cout << "Grouping is Done" << endl;
            // for(int i = 0; i < ckt->fenceCellIds.size(); ++i) {
            //     std::cout << getKeyByValue(ckt->region2id, i) << " fenceCellIds[" << i << "] size: " << ckt->fenceCellIds[i].size() << endl;
            // }
            break;
        default:
            break;
    }
    return 0;
}
int circuit::ReadDef(const std::string& defName) {
    FILE* f = NULL;
    //  long start_mem;
    int line_num_print_interval = 10000;
    fout = stdout;
    CircuitParser cp(this);
    userData = cp.Circuit();

    defrSetLogFunction(myLogFunction);

    defrInitSession(0);
    
    defrSetWarningLogFunction(myWarningLogFunction);
    //  defrSetUserData((void*)3);
    defrSetUserData(userData);
    (void)defrSetOpenLogFileAppend();

    // CircuitCallBack 
    defrSetDesignCbk(cp.DefDesignCbk);
    defrSetUnitsCbk(cp.DefUnitsCbk);
    defrSetDieAreaCbk((defrBoxCbkFnType)cp.DefDieAreaCbk);
    // rows 
    defrSetRowCbk((defrRowCbkFnType)cp.DefRowCbk);
    //DefStartCbk for memory allocation
    defrSetComponentStartCbk(cp.DefStartCbk);
    defrSetNetStartCbk(cp.DefStartCbk);
    defrSetStartPinsCbk(cp.DefStartCbk);
    defrSetGroupsStartCbk(cp.DefStartCbk);
    // Components
    defrSetComponentCbk(cp.DefComponentCbk);
    // Regions
    defrSetRegionCbk((defrRegionCbkFnType)cp.DefRegionCbk);
    // pins
    defrSetPinCbk((defrPinCbkFnType)cp.DefPinCbk);
    // Nets
    defrSetNetCbk(cp.DefNetCbk);
    // defrSetSNetWireCbk(cp.DefSNetWireCbk);
    defrSetSNetCbk(cp.DefSNetCbk);
    defrSetAddPathToNet();
    //groups
    // defrSetGroupCbk((defrGroupCbkFnType)cp.DefGroupCbk);
    defrSetGroupNameCbk((defrStringCbkFnType)cp.DefGroupNameCbk);
    defrSetGroupMemberCbk((defrStringCbkFnType)cp.DefGroupMemberCbk);
    defrSetGroupsEndCbk(cp.DefEndCbk);
    // blockages
    defrSetBlockageCbk((defrBlockageCbkFnType)cp.DefBlockageCbk);
    // End Design
    // defrSetDesignEndCbk(cp.DefEndCbk);

    defrSetWarningLogFunction(printWarning);
    defrSetLongLineNumberFunction(lineNumberCB);
    defrSetDeltaNumberLines(line_num_print_interval); 
    // File Read 
    char* fileStr = strdup(defName.c_str());
    if((f = fopen(fileStr, "r")) == 0) {
      fprintf(stderr, "**\nERROR: Couldn't open input file '%s'\n",
              fileStr);
      exit(1);
    }     

    int res = defrRead(f, fileStr, userData, 1);
    if( res ) {
      std::cout << "Reader returns bad status: " << fileStr << std::endl;
      exit(1); 
    }
    else {
      std::cout << "Reading " << fileStr << " is Done" << std::endl;  
    } 


    //// defrUnset all Cbk functions
    (void)defrPrintUnusedCallbacks(fout);
    (void)defrReleaseNResetMemory();

    (void)defrUnsetCallbacks();
    (void)defrSetUnusedCallbacks(unUsedCB);
    defrUnsetDesignCbk();
    defrUnsetUnitsCbk();
    defrUnsetDieAreaCbk();
    defrUnsetRowCbk();
    defrUnsetComponentStartCbk();
    defrUnsetNetStartCbk();
    defrUnsetComponentCbk();
    defrUnsetNetCbk();
    defrUnsetRegionCbk();
    defrUnsetGroupCbk();
    defrUnsetPinCbk();

    fclose(f);
    // Release allocated singleton data.
    defrClear();

    circuit* ckt = (circuit*) userData;
    std::cout << "Design Name: " << ckt->design_name << std::endl;

    return res;

}


void circuit::read_files(int argc, char *argv[]) 
{

    std::vector<std::string> lefStor;
    // bool doParallel;
    for(int i = 1; i < argc; i++) {
        if(i + 1 != argc) {
            if(strncmp(argv[i], "-doParallel", 11) == 0){
              if(strcmp(argv[++i], "true") == 0) {
                doParallel = true;
              }
            }
            else if(strncmp(argv[i], "-lef", 4) == 0)
                lefStor.push_back( argv[++i] );
            else if(strncmp(argv[i], "-def", 4) == 0)
                defLoc = argv[++i];
            else if(strncmp(argv[i], "-placement_constraints", 22) == 0)
                constraints = argv[++i];
            else if(strncmp(argv[i], "-output_def", 11) == 0)
                out_def = argv[++i];
        }
    }

    for(auto& curLefLoc : lefStor) {
        std::cout << " lef               : " << curLefLoc << std::endl;
    }
    std::cout << " def               : " << defLoc << std::endl;
    if(constraints != NULL)
        std::cout << " constraints       : " << constraints << std::endl;
    if(out_def != NULL)
        std::cout << " out_def           : " << out_def << std::endl;

    ReadLef(lefStor);
    ReadDef(defLoc );
}



void circuit::write_def() {
    std::string input_def_ = defLoc, output_def_ = out_def;
    if( output_def_ == "unknown" )
        output_def_ = "output/" + design_name + ".def";

    log() << "  Writing .def file : " << output_def_ << std::endl;
    
    std::ifstream in( input_def_ );
    std::ofstream out( output_def_ );

    if( !in ){
        std::cerr << "[ERROR] Cannot open file : " << input_def_ << std::endl;
        exit(0);
    }
    if( !out ){
        std::cerr << "[ERROR] Cannot open file : " << output_def_ << std::endl;
        exit(0);
    }

    const size_t maxLength = 1000000;
    char line[maxLength];
    bool isSkip = false;

    while( !in.eof() ){
        in.getline( line, maxLength );

        if( strncmp( line, "COMPONENTS", 10 ) == 0 ){
            writeComponents( out );
            isSkip = true;
        }

        if( !isSkip ) { out << line << std::endl; }

        if( strncmp( line, "END COMPONENTS", 14 ) == 0 ) 
          { isSkip = false; }
    }
    in.close();
    out.close();
    
}

void circuit::writeComponents(std::ofstream &os)
{
  os.setf( std::ios::fixed, std::ios::floatfield );

  os << "COMPONENTS " << cell_num << " ;" << std::endl;
  // sort(cells.begin(), cells.end(), cmp_id);
  double avg_disp = 0.0;
  for( size_t i = 0; i < cell_num; ++i ){
    Cell* theCell = cells[i];
    Macro* theMacro = &macros[theCell->type_];
    os << "   - " << theCell->name_ << " " << theMacro->name << std::endl;
    int ix = round(theCell->init_x_*DEFdist2Microns*min_width);
    int iy = round(theCell->init_y_*DEFdist2Microns*min_width);
    int cx = round(theCell->cur_x_*DEFdist2Microns*min_width);
    int cy = round(theCell->cur_y_*DEFdist2Microns*min_width);
#ifdef calDis
    avg_disp += fabs(ix - cx + 0.0)/DEFdist2Microns/min_width + fabs(iy - cy + 0.0)/DEFdist2Microns/min_width;
#endif
    if(theCell->isFixed_)
      os << "      + " << "FIXED ( " << ix << " " << iy << " ) " << theCell->cellorient_ << " ;" << std::endl;
    else  {
      os << "      + " << "PLACED ( " << cx << " " << cy << " ) " << theCell->cellorient_ << " ;" << std::endl; 
    } 
#ifdef debugNumErr
      if(fabs(theCell->width_ * min_width - theMacro->width) > EPSILON or fabs(theCell->height_ * min_width - theMacro->height) > EPSILON) {
      log() << "id: " << i << " (" << theCell->width_ * min_width << " " << theMacro->width << ") (" << theCell->height_ * min_width <<
      " " << theMacro->height << " )" <<endl;           
      }
#endif    
  }
#ifdef calDis
  avg_disp = avg_disp / cell_num;
  log() << "avg_disp " << avg_disp << endl; //There are numerical errors.
#endif
  os << "END COMPONENTS" << std::endl;
}


void circuit::cal_hpwl() {
    //calculate initial hpwl and legalized hpwl.
    double hpwl_gp = 0.0;
    double hpwl_lg = 0.0;
    for (Net& net : nets)
    {
        Pin& pin_source = pins[net.source];
        auto it = cell2id.find(pin_source.inst_name);
        Box<double> box_gp;
        Box<double> box_lg;
        if(it != cell2id.end()) {  //belong to Macro_pin
            Cell* cell = cells[it->second];
            Macro& theMacro = macros[cell->type_];
            Macro_pin& thePin = theMacro.pins[pin_source.port_name];
            double pin_init_x = cell->init_x_ * min_width + 0.5 * (thePin.xLL + thePin.xUR);
            double pin_init_y = cell->init_y_ * min_width + 0.5 * (thePin.yLL + thePin.yUR);
            double pin_cur_x = 0.0; 
            double pin_cur_y = 0.0;
            if(cell->isFixed_) {
                assert(cell->cellorient_ == "N");
                if(cell->cellorient_ == "N") {
                pin_cur_x = cell->init_x_ * min_width + 0.5 * (thePin.xLL + thePin.xUR);
                pin_cur_y = cell->init_y_ * min_width + 0.5 * (thePin.yLL + thePin.yUR);
                }
            }
            else {
                if(cell->cellorient_ == "N") {
                pin_cur_x = cell->cur_x_ * min_width + 0.5 * (thePin.xLL + thePin.xUR);
                pin_cur_y = cell->cur_y_ * min_width + 0.5 * (thePin.yLL + thePin.yUR);
                }
                else if(cell->cellorient_ == "FS") {
                pin_cur_x = cell->cur_x_ * min_width + 0.5 * (thePin.xLL + thePin.xUR);
                pin_cur_y = (cell->cur_y_ + cell->height_) * min_width - 0.5 * (thePin.yLL + thePin.yUR);
                }      
                else {
                std::cerr << "only  check the orient N and FS now." << std::endl;
                }        
            }

            box_gp.set(pin_init_x, pin_init_y);
            box_lg.set(pin_cur_x, pin_cur_y);
        }
        else {
            box_gp.set(pin_source.x_coord/DEFdist2Microns, pin_source.y_coord/DEFdist2Microns);
            box_lg.set(pin_source.x_coord/DEFdist2Microns, pin_source.y_coord/DEFdist2Microns);
        }
        if(net.sinks.empty()) {
            continue;
        }

        for (auto pin_id : net.sinks)
        {
            Pin& pin_sink = pins[pin_id];
            auto it = cell2id.find(pin_sink.inst_name);
            if(it != cell2id.end()) {
                Cell* cell = cells[it->second];
                Macro& theMacro = macros[cell->type_];
                Macro_pin& thePin = theMacro.pins[pin_sink.port_name];
                double pin_init_x = cell->init_x_ * min_width + 0.5 * (thePin.xLL + thePin.xUR);
                double pin_init_y = cell->init_y_ * min_width + 0.5 * (thePin.yLL + thePin.yUR);
                double pin_cur_x = 0.0;
                double pin_cur_y = 0.0;

                if(cell->isFixed_) {
                    assert(cell->cellorient_ == "N");
                    if(cell->cellorient_ == "N") {
                        pin_cur_x = cell->init_x_ * min_width + 0.5 * (thePin.xLL + thePin.xUR);
                        pin_cur_y = cell->init_y_ * min_width + 0.5 * (thePin.yLL + thePin.yUR);
                    }          
                }
                else {
                    if(cell->cellorient_ == "N") {
                        pin_cur_x = cell->cur_x_ * min_width + 0.5 * (thePin.xLL + thePin.xUR);
                        pin_cur_y = cell->cur_y_ * min_width + 0.5 * (thePin.yLL + thePin.yUR);
                    }
                    else if(cell->cellorient_ == "FS"){
                        pin_cur_x = cell->cur_x_ * min_width + 0.5 * (thePin.xLL + thePin.xUR);
                        pin_cur_y = (cell->cur_y_ + cell->height_) * min_width - 0.5 * (thePin.yLL + thePin.yUR);
                    }   
                    else {
                        std::cerr << "only  check the orient N and FS now." << std::endl;
                    }            
                }        
                box_gp.fupdate(pin_init_x, pin_init_y);
                box_lg.fupdate(pin_cur_x, pin_cur_y);
            }
            else {
                box_gp.fupdate(pin_sink.x_coord/DEFdist2Microns, pin_sink.y_coord/DEFdist2Microns);
                box_lg.fupdate(pin_sink.x_coord/DEFdist2Microns, pin_sink.y_coord/DEFdist2Microns);
            }
        }
        hpwl_gp += box_gp.hp();
        hpwl_lg += box_lg.hp();
    }
    double delta_rate = 100.0 * (hpwl_lg - hpwl_gp) / hpwl_gp;
    log() << "  hpwl_gp : " << hpwl_gp << " hpwl_lg : " << hpwl_lg << " delta_rate : " << delta_rate << "%" << std::endl;
    if(doTech) {
        log() << "score : " << (1.0 + delta_rate / 100.0 + (Np + Ne + 0.0) / cell_num) * (1.0 + max_disp/defaultH/100.0) * s_am / defaultH << std::endl;
    }
}

