#include <parser.h>

static int ccr1131444 = 0;
void printWarning(const char *str)
{
  fprintf(stderr, "%s\n", str);
}
void lineNumberCB(int lineNo) {
//   cout << "Parsed " << lineNo << " number of lines!!" << endl;
}
void errorCB(const char* msg) {
  std::cout << msg << std::endl;
}
void warningCB(const char* msg) {
  std::cout << msg << std::endl;
}
void myLogFunction(const char* errMsg) {
  fprintf(fout, "ERROR: found error: %s\n", errMsg);
}
void myWarningLogFunction(const char* errMsg) {
  fprintf(fout, "WARNING: found error: %s\n", errMsg);
}

void lineNumberCB(long long lineNo) {
  // The CCR 1131444 tests ability of the DEF parser to count
  // input line numbers out of 32-bit int range. On the first callback
  // call 10G lines will be added to line counter. It should be
  // reflected in output.
  if(ccr1131444) {
    lineNo += 10000000000LL;
    defrSetNLines(lineNo);
    ccr1131444 = 0;
  }
//   cout << "Parsed " << lineNo << " number of lines!!" << endl;
}

int unUsedCB(defrCallbackType_e, void*, defiUserData) {
  fprintf(fout, "This callback is not used.\n");
  return 0;
}

void printMacro(defiUserData ud, const std::string& name)
{
    circuit* ckt = (circuit*) ud;
    auto it = find_if(ckt->macros.begin(), ckt->macros.end(), [name](const Macro& macro) {
        return macro.name == name;
    });
    Macro& macor = *it;
    std::cout << "Macro: " << macor.name << " type: " << macor.type << " xOrig: " << macor.xOrig << " yOrig: " << macor.yOrig << " width: " << macor.width << " height: " << macor.height << " numVDD: " << macor.numVDD << " numVSS: " << macor.numVSS << " isFlop: " << macor.isFlop << " aligendRow: " << macor.aligendRow << std::endl;
    for(auto& pin: macor.pins) {
        std::cout << "Pin: " << pin.first << " direction: " << pin.second.direction << " layer: " << pin.second.layer << " xLL: " << pin.second.xLL << " yLL: " << pin.second.yLL << " xUR: " << pin.second.xUR << " yUR: " << pin.second.yUR << " xLL_offset_psite: " << pin.second.xLL_offset_psite << " xUR_offset_psite: " << pin.second.xUR_offset_psite << " yLL_offset_psite: " << pin.second.yLL_offset_psite << " yUR_offset_psite: " << pin.second.yUR_offset_psite << std::endl;
    }
    for(auto& obs: macor.obses) {
        std::cout << "Obs: xLL: " << obs.xLL_ << " yLL: " << obs.yLL_ << " xUR: " << obs.xUR_ << " yUR: " << obs.yUR_ << std::endl;
    }

}

std::string getKeyByValue(std::map<std::string, unsigned>& map, unsigned value) {
    for (const auto& pair : map) {
        if (pair.second == value) {
            return pair.first;
        }
    }
    return "";
}


site *circuit::locateOrCreateSite(const std::string &siteName) {
//   unordered_map< string, unsigned >::iterator it = site2id.find(siteName);
//   if(it == site2id.end()) {
    site theSite;
    theSite.name = siteName;
    site2id.insert(make_pair(theSite.name, sites.size()));
    sites.push_back(theSite);
    return &sites[sites.size() - 1];
//   }
//   else
//     return &sites[it->second];
}

layer *circuit::locateOrCreateLayer(const std::string &layerName) {
  std::map< std::string, unsigned >::iterator it = layer2id.find(layerName);
  if(it == layer2id.end()) {
    layer theLayer;
    theLayer.name = layerName;
    layer2id.insert(make_pair(theLayer.name, layers.size()));
    layers.push_back(theLayer);
    return &layers[layers.size() - 1];
  }
  else
    return &layers[it->second];
}

Macro *circuit::locateOrCreateMacro(const std::string &macroName) {
//   unordered_map< string, unsigned >::iterator it = macro2id.find(macroName);
//   if(it == macro2id.end()) {
    Macro theMacro;
    theMacro.name = macroName;
    macro2id.insert(make_pair(theMacro.name, macros.size()));
    macros.push_back(theMacro);
    return &macros[macros.size() - 1];
//   }
//   else
//     return &macros[it->second];
}

Row *circuit::locateOrCreateRow(const std::string &rowName) {
//   unordered_map< string, unsigned >::iterator it = row2id.find(rowName);
//   if(it == row2id.end()) {
    Row theRow;
    theRow.name = rowName;
    row2id.insert(make_pair(theRow.name, rows.size()));
    rows.push_back(theRow);
    return &rows[rows.size() - 1];
//   }
//   else
//     return &prevrows[it->second];
}

Cell *circuit::locateOrCreateCell(const std::string &cellName) {
//   unordered_map< string, unsigned >::iterator it = cell2id.find(cellName);
//   if(it == cell2id.end()) {
    Cell *theCell = new Cell();
    theCell->name_ = cellName;
    cell2id.insert(make_pair(theCell->name_, cells.size()));
    cells.push_back(theCell);
    // return cells[cells.size()-1];
    return theCell;
//   }
//   else
//     return &cells[it->second];
}

FenceRegion* circuit::locateOrCreateRegion(const std::string &regionName) {
  //assert this is a new fenceRegion
  FenceRegion* fenceRegion = new FenceRegion();
  fenceRegion->name_ = regionName;
  fenceRegion->id_ = fenceRegions.size();
  region2id.insert(make_pair(fenceRegion->name_, fenceRegions.size()));
  fenceRegions.push_back(fenceRegion);
  return fenceRegion;
}


// requires full name, e.g. cell_instance/pin
Pin* circuit::locateOrCreatePin(const std::string &instName, const std::string &portName)
{
  std::map<std::string, unsigned>::iterator it = pin2id.find(instName);
  if (it == pin2id.end())
  {
    Pin thePin;
    thePin.inst_name = instName;
    thePin.port_name = portName;
    std::string all_name;
    thePin.id   = pins.size();
    if(portName == "") {
      pin2id.insert(make_pair(instName, thePin.id));
    } else {
      all_name = instName + "_" + portName;
      pin2id.insert(make_pair(all_name, thePin.id));
    }

    pins.push_back(thePin);
    return &pins[pins.size()-1];
  }
  else
    return &pins[it->second];
}


Net *circuit::locateOrCreateNet(const std::string &netName) {
//   map< string, unsigned >::iterator it = net2id.find(netName);
//   if(it == net2id.end()) {
    Net theNet;
    theNet.name = netName;
    net2id.insert(make_pair(theNet.name, nets.size()));
    nets.push_back(theNet);
    return &nets[nets.size() - 1];
//   }
//   else
//     return &nets[it->second];
}


void circuit::setMaxXY() { 
    g_max_x = round(round(rx)/min_width);
    g_max_y = round(round(rx)/min_width);
    g_max_y = g_max_y / defaultH * defaultH;
}

