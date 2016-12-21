#pragma once
#include <mxml.h>
#include <string>
#include <assert.h>
#include <vector>

struct XElement { //XMLAttribute
	std::string elementName;
	std::vector<std::string> attributeName;
	std::vector<std::string> attributeValue;
	XElement(std::string name, std::vector<std::string> attribName, std::vector<std::string> values)  : elementName(name)
	, attributeName(), attributeValue()
	{
		for (size_t i = 0; i < attribName.size();i++) {
			attributeName.push_back(attribName[i]);
			attributeValue.push_back(values[i]);
		}
	}
	XElement() : elementName(), attributeValue(), attributeName() {}
	std::string to_string(std::string separator = " ") {
		std::string s = "";
		s = elementName;
		s += " : ";
		for (size_t i = 0; i < attributeName.size();i++) {
			s += attributeName[i];
			s += "(";
			s += attributeValue[i];
			s += ") ";
		}
		
		return s;
	}

};

class XMLParser
{
public:
	XMLParser(std::string filename);
	~XMLParser();

	std::string to_string(); //Use for check f the xml file is well parsed
	mxml_node_t* getNext(mxml_node_t* node); //Get The next node
	mxml_node_t* getNextElement(mxml_node_t *node); //Get the next element
	mxml_node_t* getTop(); //Get the top node of the file
	XElement getXData(mxml_node_t* node); //Extract XElement from a nodes
	std::string getValue(std::string elementName, std::string attributeName); //explicit
	std::vector<mxml_node_t*> getChilds(mxml_node_t* parent);
protected:
	FILE *m_file;
	std::string m_filename;
	mxml_node_t* m_tree;
};

