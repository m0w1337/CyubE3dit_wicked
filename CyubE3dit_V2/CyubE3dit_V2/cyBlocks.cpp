#include "stdafx.h"
#include "cyBlocks.h"
#include "json.hpp"
#include <sstream>
#include <string>

using namespace std;
using json = nlohmann::json;

void cyBlocks::LoadRegBlocks(void) {
	ifstream file;
	file.open("data/blocks.json", fstream::in);
	if (file.is_open()) {
		json j_complete = json::parse(file);
		
		for (json::iterator it = j_complete.begin(); it != j_complete.end(); ++it) {
			if (it.key() == "Block") {
				it.value().end();
				if (it.value().is_array()) {
					for (size_t i = 0; i < it.value().size();i++) {
						json::iterator id =  it.value().at(i).find("id"); 
						if (id  != it.value().at(i).end()) {
							m_regBlockTypes[(uint8_t) stoi((string) id.value(),nullptr,10)] = BLOCKTYPE_BLOCK;
						}
					}
				}
			}
			//it.at("address").get_to(p.address);
			//it.at("age").get_to(p.age);
		}
	
	}
	
}
