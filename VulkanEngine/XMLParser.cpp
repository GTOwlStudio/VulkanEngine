#include "XMLParser.h"



XMLParser::XMLParser(std::string filename) : m_filename(filename)
{
	//FILE* file;
	m_file = fopen(m_filename.c_str(), "r");
	assert(m_file);
	m_tree = mxmlLoadFile(NULL, m_file, MXML_TEXT_CALLBACK);
	
}


XMLParser::~XMLParser()
{
	fclose(m_file);
}

std::string XMLParser::to_string()
{
	std::string s = "";
	mxml_node_t* node = m_tree;//mxmlWalkNext(m_tree, m_tree, MXML_DESCEND);
	node = mxmlGetFirstChild(node);
	XElement tmp;
	while(node!=nullptr) 
	{
		//node = getNext(node);
		//if (mxmlGetType(node)==MXML_ELEMENT) {
		node = getNextElement(node);
		if (node==nullptr) {
			break;
		}
		//s += mxmlGetElement(node);
		tmp = getXData(node);
		s += tmp.to_string();
		s += "\n";
	//	}
	}
	return s;
}

mxml_node_t * XMLParser::getNext(mxml_node_t * n)
{
	if (mxmlGetFirstChild(n) != NULL) {
		return mxmlGetFirstChild(n);
	}
	else if (mxmlGetNextSibling(n) != NULL) {
		return mxmlGetNextSibling(n);
	}
	else if (mxmlGetNextSibling(mxmlGetParent(n)) != NULL) {
		return mxmlGetNextSibling(mxmlGetParent(n));
	}
	return nullptr;
}

mxml_node_t * XMLParser::getTop()
{
	return getNextElement(m_tree);
}

mxml_node_t* XMLParser::getNextElement(mxml_node_t * node)
{
	mxml_node_t* n = getNext(node);
	while ((n!=nullptr)&&(mxmlGetType(n)!=MXML_ELEMENT)) {
		n = getNext(n);
	}
	return n;
}

XElement XMLParser::getXData(mxml_node_t * node)
{


	if ((node==nullptr)||(mxmlGetType(node)!=MXML_ELEMENT)) {
		return XElement();
	}

	std::vector<std::string> names;
	std::vector<std::string> values;
	for (int i = 0; i < node->value.element.num_attrs;i++) {
		names.push_back(node->value.element.attrs[i].name);
		values.push_back(node->value.element.attrs[i].value);
	}

	return XElement(mxmlGetElement(node),names, values);

	//return elem;
}

std::string XMLParser::getValue(std::string elementName, std::string attributeName)
{
	std::string value="empty:string";
	mxml_node_t* node = getNextElement(m_tree);
	while (node != nullptr) {
		if (std::string(node->value.element.name) == elementName) {
			for (int i = 0; i < node->value.element.num_attrs; i++) {
				if (std::string(node->value.element.attrs[i].name) == attributeName) {
					value = node->value.element.attrs[i].value;
					break;
				}
			}
			break;
		}
		node = getNextElement(node);
	}
	return value;
}

std::vector<mxml_node_t*> XMLParser::getChilds(mxml_node_t * parent)
{
	std::vector<mxml_node_t*> childs;
	mxml_node_t* node = mxmlGetFirstChild(parent);
	while (node!=nullptr) {
		if (mxmlGetType(node)==MXML_ELEMENT) {
			childs.push_back(node);
		}
		node = mxmlGetNextSibling(node);
	}
	return childs;
}
