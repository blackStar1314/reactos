
#include "../../pch.h"

#include "mingw.h"
#include <assert.h>

using std::string;
using std::vector;

static class MingwFactory : public Backend::Factory
{
public:
	MingwFactory() : Factory ( "mingw" ) {}
	Backend* operator() ( Project& project )
	{
		return new MingwBackend ( project );
	}
} factory;


MingwBackend::MingwBackend ( Project& project )
	: Backend ( project )
{
}

void
MingwBackend::Process ()
{
	CreateMakefile ();
	GenerateHeader ();
	GenerateGlobalVariables ();
	GenerateAllTarget ();
	for ( size_t i = 0; i < ProjectNode.modules.size (); i++ )
	{
		Module& module = *ProjectNode.modules[i];
		ProcessModule ( module );
	}
	CloseMakefile ();
}

void
MingwBackend::CreateMakefile ()
{
	fMakefile = fopen ( ProjectNode.makefile.c_str (), "w" );
	if ( !fMakefile )
		throw AccessDeniedException ( ProjectNode.makefile );
	MingwModuleHandler::SetMakefile ( fMakefile );
}

void
MingwBackend::CloseMakefile () const
{
	if (fMakefile)
		fclose ( fMakefile );
}

void
MingwBackend::GenerateHeader () const
{
	fprintf ( fMakefile, "# THIS FILE IS AUTOMATICALLY GENERATED, EDIT 'ReactOS.xml' INSTEAD\n\n" );
}

void
MingwBackend::GenerateProjectCFlagsMacro ( const char* assignmentOperation,
                                           const vector<Include*>& includes,
                                           const vector<Define*>& defines ) const
{
	size_t i;

	fprintf (
		fMakefile,
		"PROJECT_CFLAGS %s",
		assignmentOperation );
	for ( i = 0; i < includes.size(); i++ )
	{
		fprintf (
			fMakefile,
			" -I%s",
			includes[i]->directory.c_str() );
	}
	
	for ( i = 0; i < defines.size(); i++ )
	{
		Define& d = *defines[i];
		fprintf (
			fMakefile,
			" -D%s",
			d.name.c_str() );
		if ( d.value.size() )
			fprintf (
				fMakefile,
				"=%s",
				d.value.c_str() );
	}
	fprintf ( fMakefile, "\n" );
}

void
MingwBackend::GenerateGlobalCFlagsAndProperties (
	const char* assignmentOperation,
	const vector<Property*>& properties,
	const vector<Include*>& includes,
	const vector<Define*>& defines,
	const vector<If*>& ifs ) const
{
	size_t i;

	for ( i = 0; i < properties.size(); i++ )
	{
		Property& prop = *properties[i];
		fprintf ( fMakefile, "%s := %s\n",
			prop.name.c_str(),
			prop.value.c_str() );
	}

	if ( includes.size() || defines.size() )
	{
		GenerateProjectCFlagsMacro ( assignmentOperation,
                                     includes,
                                     defines );
	}

	for ( i = 0; i < ifs.size(); i++ )
	{
		If& rIf = *ifs[i];
		if ( rIf.defines.size() || rIf.includes.size() || rIf.ifs.size() )
		{
			fprintf (
				fMakefile,
				"ifeq (\"$(%s)\",\"%s\")\n",
				rIf.property.c_str(),
				rIf.value.c_str() );
			GenerateGlobalCFlagsAndProperties (
				"+=",
				rIf.properties,
				rIf.includes,
				rIf.defines,
				rIf.ifs );
			fprintf ( 
				fMakefile,
				"endif\n\n" );
		}
	}
}

string
MingwBackend::GenerateProjectLFLAGS () const
{
	string lflags;
	for ( size_t i = 0; i < ProjectNode.linkerFlags.size (); i++ )
	{
		LinkerFlag& linkerFlag = *ProjectNode.linkerFlags[i];
		if ( lflags.length () > 0 )
			lflags += " ";
		lflags += linkerFlag.flag;
	}
	return lflags;
}

void
MingwBackend::GenerateGlobalVariables () const
{
#ifdef WIN32
	fprintf ( fMakefile, "host_gcc = gcc\n" );
	fprintf ( fMakefile, "host_gpp = g++\n" );
	fprintf ( fMakefile, "host_ld = ld\n" );
	fprintf ( fMakefile, "host_ar = ar\n" );
	fprintf ( fMakefile, "host_objcopy = objcopy\n" );
	fprintf ( fMakefile, "rm = del /f /q\n" );
	fprintf ( fMakefile, "gcc = gcc\n" );
	fprintf ( fMakefile, "gpp = g++\n" );
	fprintf ( fMakefile, "ld = ld\n" );
	fprintf ( fMakefile, "ar = ar\n" );
	fprintf ( fMakefile, "objcopy = objcopy\n" );
	fprintf ( fMakefile, "dlltool = dlltool\n" );
	fprintf ( fMakefile, "windres = windres\n" );
#else
	fprintf ( fMakefile, "host_gcc = gcc\n" );
	fprintf ( fMakefile, "host_gpp = g++\n" );
	fprintf ( fMakefile, "host_ld = ld\n" );
	fprintf ( fMakefile, "host_ar = ar\n" );
	fprintf ( fMakefile, "host_objcopy = objcopy\n" );
	fprintf ( fMakefile, "rm = rm -f\n" );
	fprintf ( fMakefile, "gcc = mingw32-gcc\n" );
	fprintf ( fMakefile, "gpp = mingw32-g++\n" );
	fprintf ( fMakefile, "ld = mingw32-ld\n" );
	fprintf ( fMakefile, "ar = mingw32-ar\n" );
	fprintf ( fMakefile, "objcopy = mingw32-objcopy\n" );
	fprintf ( fMakefile, "dlltool = mingw32-dlltool\n" );
	fprintf ( fMakefile, "windres = mingw32-windres\n" );
#endif
	fprintf ( fMakefile, "mkdir = tools%crmkdir\n", CSEP );
	fprintf ( fMakefile, "NUL=NUL\n" );
	fprintf ( fMakefile, "winebuild = tools" SSEP "winebuild" SSEP "winebuild\n" );
	fprintf ( fMakefile, "bin2res = tools" SSEP "bin2res" SSEP "bin2res\n" );
	fprintf ( fMakefile, "cabman = tools" SSEP "cabman" SSEP "cabman\n" );
	fprintf ( fMakefile, "cdmake = tools" SSEP "cdmake" SSEP "cdmake\n" );
	fprintf ( fMakefile, "\n" );
	GenerateGlobalCFlagsAndProperties (
		"=",
		ProjectNode.properties,
		ProjectNode.includes,
		ProjectNode.defines,
		ProjectNode.ifs );
	fprintf ( fMakefile, "PROJECT_RCFLAGS = $(PROJECT_CFLAGS)\n" );
	fprintf ( fMakefile, "PROJECT_LFLAGS = %s\n",
	          GenerateProjectLFLAGS ().c_str () );
	fprintf ( fMakefile, "\n" );

	fprintf ( fMakefile, ".PHONY: clean\n\n" );
}

bool
MingwBackend::IncludeInAllTarget ( const Module& module ) const
{
	if ( module.type == ObjectLibrary )
		return false;
	if ( module.type == BootSector )
		return false;
	if ( module.type == Iso )
		return false;
	return true;
}

void
MingwBackend::GenerateAllTarget () const
{
	fprintf ( fMakefile, "all:" );
	for ( size_t i = 0; i < ProjectNode.modules.size (); i++ )
	{
		Module& module = *ProjectNode.modules[i];
		if ( IncludeInAllTarget ( module ) )
		{
			fprintf ( fMakefile,
			          " %s",
			          FixupTargetFilename ( module.GetPath () ).c_str () );
		}
	}
	fprintf ( fMakefile, "\n\t\n\n" );
}

void
MingwBackend::ProcessModule ( Module& module ) const
{
	MingwModuleHandler* h = MingwModuleHandler::LookupHandler (
		module.node.location,
		module.type );
	h->Process ( module );
	h->GenerateDirectoryTargets ();
}

string
FixupTargetFilename ( const string& targetFilename )
{
	return string("$(ROS_INTERMEDIATE)") + NormalizeFilename ( targetFilename );
}
