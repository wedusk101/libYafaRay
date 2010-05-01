#include <yafray_config.h>
#include <cstdlib>
#include <iostream>
#include <cctype>
#include <algorithm>

#ifdef WIN32
	#ifndef __MINGW32__
		#define NOMINMAX
	#endif
	#include <windows.h>
#endif

#include <core_api/scene.h>
#include <core_api/environment.h>
#include <core_api/integrator.h>
#include <core_api/imagefilm.h>
#include <yafraycore/tga_io.h>
#include <yafraycore/EXR_io.h>
#include <yafraycore/xmlparser.h>
#include <yaf_revision.h>
#include <utilities/console_utils.h>
#include <yafraycore/imageOutput.h>

#include "../gui/yafqtapi.h"

using namespace::yafaray;

int main(int argc, char *argv[])
{
	cliParser_t parse(argc, argv, 2, 1, "You need to set at least a yafaray's valid XML file.");
	
#ifdef RELEASE
	std::string version = std::string(VERSION);
#else
	std::string version = std::string(YAF_SVN_REV);
#endif

	std::string xmlLoaderVersion = "YafaRay XML loader version 0.2";

	renderEnvironment_t *env = new renderEnvironment_t();
	
	// Plugin load
	std::string ppath = "";
	
	if (env->getPluginPath(ppath))
	{
		Y_DEBUG(1) << "The plugin path is: " << ppath << std::endl;
		env->loadPlugins(ppath);
	}
	else
	{
		Y_ERROR << "Getting plugin path from render environment failed!" << std::endl;
		return 1;
	}
	
	std::vector<std::string> formats = env->listImageHandlers();
	
	std::string formatString = "";
	for(size_t i = 0; i < formats.size(); i++)
	{
		formatString.append("                                       " + formats[i]);
		if(i < formats.size() - 1) formatString.append("\n");
	}

	parse.setAppName(xmlLoaderVersion,
	"[OPTIONS]... <input xml file> [output filename]\n<input xml file> : A valid yafaray XML file\n[output filename] : The filename of the rendered image without extension.\n*Note: If output filename is ommited the name \"yafaray\" will be used instead.");
	// Configuration of valid flags
	
	parse.setOption("v","version", true, "Displays this program's version.");
	parse.setOption("h","help", true, "Displays this help text.");
	parse.setOption("op","output-path", false, "Uses the path in <value> as rendered image output path.");
	parse.setOption("f","format", false, "Sets the output image format, available formats are:\n\n" + formatString + "\n                                       Default: tga.\n");
	parse.setOption("t","threads", false, "Overrides threads setting on the XML file, for auto selection use -1.");
	parse.setOption("a","with-alpha", true, "Enables saving the image with alpha channel.");
	parse.setOption("dp","draw-params", true, "Enables saving the image with a settings badge.");
	parse.setOption("ndp","no-draw-params", true, "Disables saving the image with a settings badge (warning: this overrides --draw-params setting).");
	parse.setOption("cs","custom-string", false, "Sets the custom string to be used on the settings badge.");
	parse.setOption("z","z-buffer", true, "Enables the rendering of the depth map (Z-Buffer) (this flag overrides XML setting).");
	parse.setOption("nz","no-z-buffer", true, "Disables the rendering of the depth map (Z-Buffer) (this flag overrides XML setting).");
	
	bool parseOk = parse.parseCommandLine();
	
	if(parse.getFlag("h"))
	{
		parse.printUsage();
		return 0;
	}
	
	if(parse.getFlag("v"))
	{
		Y_INFO << xmlLoaderVersion << std::endl << "Built with YafaRay version " << version << std::endl;
		return 0;
	}
	
	if(!parseOk)
	{
		parse.printError();
		parse.printUsage();
		return 0;
	}
	
	bool alpha = parse.getFlag("a");
	std::string format = parse.getOptionString("f");
	std::string outputPath = parse.getOptionString("op");
	int threads = parse.getOptionInteger("t");
	bool drawparams = parse.getFlag("dp");
	bool nodrawparams = parse.getFlag("ndp");
	std::string customString = parse.getOptionString("cs");
	bool zbuf = parse.getFlag("z");
	bool nozbuf = parse.getFlag("nz");
	
	if(format.empty()) format = "tga";
	bool formatValid = false;
	
	for(size_t i = 0; i < formats.size(); i++)
	{
		if(formats[i].find(format) != std::string::npos) formatValid = true;
	}
	
	if(!formatValid)
	{
		Y_ERROR << "Couldn't find any valid image format, image handlers missing?" << std::endl;
		return 1;
	}
	
	const std::vector<std::string> files = parse.getCleanArgs();
	
	if(files.size() == 0)
	{
		return 0;
	}
	
	std::string outName = "yafray." + format;
	
	if(files.size() > 1) outName = files[1] + "." + format;
	
	std::string xmlFile = files[0];
	
	//env->Debug = debug; //disabled until proper debugging messages are set throughout the core

	// Set the full output path with filename
	if (outputPath.empty())
	{
		outputPath = outName;
	}
	else if (outputPath.at(outputPath.length() - 1) == '/')
	{
		outputPath += outName;
	}
	else if (outputPath.at(outputPath.length() - 1) != '/')
	{
		outputPath += "/" + outName;
	}
	
	scene_t *scene = new scene_t();
	env->setScene(scene);
	paraMap_t render;
	
	bool success = parse_xml_file(xmlFile.c_str(), scene, env, render);
	if(!success) exit(1);
	
	int width=320, height=240;
	render.getParam("width", width); // width of rendered image
	render.getParam("height", height); // height of rendered image
	
	if(threads >= -1) render["threads"] = threads;
	
	if(drawparams)
	{
		render["drawParams"] = true;
		if(!customString.empty()) render["customString"] = customString;
	}
	
	if(nodrawparams) render["drawParams"] = false;
	
	if(zbuf) render["z_channel"] = true;
	if(nozbuf) render["z_channel"] = false;
	
	bool use_zbuf = false;
	render.getParam("z_channel", use_zbuf);
	
	// create output
	colorOutput_t *out=0;

	paraMap_t ihParams;
	ihParams["type"] = format;
	ihParams["width"] = width;
	ihParams["height"] = height;
	ihParams["alpha_channel"] = alpha;
	ihParams["z_channel"] = use_zbuf;
	
	imageHandler_t *ih = env->createImageHandler("outFile", ihParams);
	
	//testing reading
	
	ihParams["type"] = std::string("hdr");
	ihParams["width"] = width;
	ihParams["height"] = height;
	ihParams["alpha_channel"] = false;
	ihParams["z_channel"] = false;
	
	imageHandler_t *ihtest = env->createImageHandler("testFile", ihParams);
	ihtest->loadFromFile("whatever-you-want.hdr");
	ihtest->saveToFile("testfile.hdr");
	
	if(ih)
	{
		out = new imageOutput_t(ih, outputPath);
		if(!out) return 1;				
	}
	else return 1;
	
	if(! env->setupScene(*scene, render, *out) ) return 1;
	
	scene->render();
	env->clearAll();

	imageFilm_t *film = scene->getImageFilm();

	delete film;
	delete out;
	
	return 0;
}