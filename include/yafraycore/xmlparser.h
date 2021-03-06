#pragma once

#ifndef Y_XMLPARSER_H
#define Y_XMLPARSER_H

#include <yafray_constants.h>
#include <core_api/params.h>
#include <list>
#include <vector>
#include <string>

__BEGIN_YAFRAY

class scene_t;
class renderEnvironment_t;
class xmlParser_t;
enum colorSpaces_t : int;

YAFRAYCORE_EXPORT bool parse_xml_file(const char *filename, scene_t *scene, renderEnvironment_t *env, paraMap_t &render, std::string color_space_string, float input_gamma);

typedef void (*startElement_cb)(xmlParser_t &p, const char *element, const char **attrs);
typedef void (*endElement_cb)(xmlParser_t &p, const char *element);

#if HAVE_XML
struct parserState_t
{
	startElement_cb start;
	endElement_cb end;
	void *userdata;
	int level;
	std::string last_section; //! to show last section previous to a XML Parser error
	std::string last_element; //! to show last element previous to a XML Parser error
	std::string last_element_attrs; //! to show last element attributes previous to a XML Parser error
};

class xmlParser_t
{
	public:
		xmlParser_t(renderEnvironment_t *renv, scene_t *sc, paraMap_t &r, colorSpaces_t input_color_space, float input_gamma);
		void pushState(startElement_cb start, endElement_cb end, void *userdata=nullptr);
		void popState();
		void startElement(const char *element, const char **attrs){ ++level; if(current) current->start(*this, element, attrs); }
		void endElement(const char *element)	{ if(current) current->end(*this, element); --level; }
		void* stateData(){ return current->userdata; }
		void setParam(const std::string &name, parameter_t &param){ (*cparams)[name] = param; }
		int currLevel() const{ return level; }
		int stateLevel() const { return current ? current->level : -1; }
		colorSpaces_t getInputColorSpace() const { return inputColorSpace; }
		float getInputGamma() const { return inputGamma; }
		void setLastSection(const std::string &section) { current->last_section = section; }
		void setLastElementName(const char *element_name);
		void setLastElementNameAttrs(const char **element_attrs);
		std::string getLastSection() const { return current->last_section; }
		std::string getLastElementName() const { return current->last_element; }
		std::string getLastElementNameAttrs() const { return current->last_element_attrs; }
		
		renderEnvironment_t *env;
		scene_t *scene;
		paraMap_t params, &render;
		std::list<paraMap_t> eparams; //! for materials that need to define a whole shader tree etc.
		paraMap_t *cparams; //! just a pointer to the current paramMap, either params or a eparams element
	protected:
		std::vector<parserState_t> state_stack;
		parserState_t *current;
		int level;
		float inputGamma;
		colorSpaces_t inputColorSpace;
};

// state callbacks:
void startEl_document(xmlParser_t &p, const char *element, const char **attrs);
void endEl_document(xmlParser_t &p, const char *element);
void startEl_scene(xmlParser_t &p, const char *element, const char **attrs);
void endEl_scene(xmlParser_t &p, const char *element);
void startEl_mesh(xmlParser_t &p, const char *element, const char **attrs);
void endEl_mesh(xmlParser_t &p, const char *element);
void startEl_instance(xmlParser_t &p, const char *element, const char **attrs);
void endEl_instance(xmlParser_t &p, const char *element);
void startEl_parammap(xmlParser_t &p, const char *element, const char **attrs);
void endEl_parammap(xmlParser_t &p, const char *element);
void startEl_paramlist(xmlParser_t &p, const char *element, const char **attrs);
void endEl_paramlist(xmlParser_t &p, const char *element);
void endEl_render(xmlParser_t &p, const char *element);
void startEl_curve(xmlParser_t &p, const char *element, const char **attrs);
void endEl_curve(xmlParser_t &p, const char *element);

#endif // HAVE_XML

__END_YAFRAY

#endif // Y_XMLPARSER_H
