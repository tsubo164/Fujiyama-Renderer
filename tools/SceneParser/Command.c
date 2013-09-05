/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Command.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int print_property_list(const char *type_name);

/* OpenPlugin */
static const int OpenPlugin_args[] = {
  ARG_COMMAND_NAME,
  ARG_FILE_PATH};
static struct CommandResult OpenPlugin_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiOpenPlugin(args[1].str);
  return result;
}

/* RenderScene */
static const int RenderScene_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID};
static struct CommandResult RenderScene_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiRenderScene(args[1].id);
  return result;
}

/* RunProcedure */
static const int RunProcedure_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID};
static struct CommandResult RunProcedure_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiRunProcedure(args[1].id);
  return result;
}

/* SaveFrameBuffer */
static const int SaveFrameBuffer_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_FILE_PATH};
static struct CommandResult SaveFrameBuffer_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiSaveFrameBuffer(args[1].id, args[2].str);
  return result;
}

/* AddObjectToGroup */
static const int AddObjectToGroup_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_ENTRY_ID};
static struct CommandResult AddObjectToGroup_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiAddObjectToGroup(args[1].id, args[2].id);
  return result;
}

/* NewObjectInstance */
static const int NewObjectInstance_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_ENTRY_ID};
static struct CommandResult NewObjectInstance_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewObjectInstance(args[2].id);
  result.new_entry_name = args[1].str;
  return result;
}

/* NewFrameBuffer */
static const int NewFrameBuffer_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_STRING};
static struct CommandResult NewFrameBuffer_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewFrameBuffer(args[2].str);
  result.new_entry_name = args[1].str;
  return result;
}

/* NewObjectGroup */
static const int NewObjectGroup_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static struct CommandResult NewObjectGroup_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewObjectGroup();
  result.new_entry_name = args[1].str;
  return result;
}

/* NewPointCloud */
static const int NewPointCloud_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_FILE_PATH};
static struct CommandResult NewPointCloud_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewPointCloud(args[2].str);
  result.new_entry_name = args[1].str;
  return result;
}

/* NewTurbulence */
static const int NewTurbulence_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static struct CommandResult NewTurbulence_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewTurbulence();
  result.new_entry_name = args[1].str;
  return result;
}

/* NewProcedure */
static const int NewProcedure_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_STRING};
static struct CommandResult NewProcedure_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewProcedure(args[2].str);
  result.new_entry_name = args[1].str;
  return result;
}

/* NewRenderer */
static const int NewRenderer_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static struct CommandResult NewRenderer_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewRenderer();
  result.new_entry_name = args[1].str;
  return result;
}

/* NewTexture */
static const int NewTexture_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_FILE_PATH};
static struct CommandResult NewTexture_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewTexture(args[2].str);
  result.new_entry_name = args[1].str;
  return result;
}

/* NewShader */
static const int NewShader_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_STRING};
static struct CommandResult NewShader_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewShader(args[2].str);
  result.new_entry_name = args[1].str;
  return result;
}

/* NewCamera */
static const int NewCamera_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_STRING};
static struct CommandResult NewCamera_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewCamera(args[2].str);
  result.new_entry_name = args[1].str;
  return result;
}

/* NewVolume */
static const int NewVolume_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static struct CommandResult NewVolume_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewVolume();
  result.new_entry_name = args[1].str;
  return result;
}

/* NewCurve */
static const int NewCurve_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_FILE_PATH};
static struct CommandResult NewCurve_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewCurve(args[2].str);
  result.new_entry_name = args[1].str;
  return result;
}

/* NewLight */
static const int NewLight_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_LIGHT_TYPE};
static struct CommandResult NewLight_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewLight(args[2].num);
  result.new_entry_name = args[1].str;
  return result;
}

/* NewMesh */
static const int NewMesh_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_FILE_PATH};
static struct CommandResult NewMesh_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.new_entry_id = SiNewMesh(args[2].str);
  result.new_entry_name = args[1].str;
  return result;
}

/* AssignFrameBuffer */
static const int AssignFrameBuffer_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_ENTRY_ID};
static struct CommandResult AssignFrameBuffer_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiAssignFrameBuffer(args[1].id, args[2].id);
  return result;
}

/* AssignObjectGroup */
static const int AssignObjectGroup_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static struct CommandResult AssignObjectGroup_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiAssignObjectGroup(args[1].id, args[2].str, args[3].id);
  return result;
}

/* AssignTurbulence */
static const int AssignTurbulence_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static struct CommandResult AssignTurbulence_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiAssignTurbulence(args[1].id, args[2].str, args[3].id);
  return result;
}

/* AssignTexture */
static const int AssignTexture_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static struct CommandResult AssignTexture_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiAssignTexture(args[1].id, args[2].str, args[3].id);
  return result;
}

/* AssignCamera */
static const int AssignCamera_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_ENTRY_ID};
static struct CommandResult AssignCamera_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiAssignCamera(args[1].id, args[2].id);
  return result;
}

/* AssignShader */
static const int AssignShader_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_ENTRY_ID};
static struct CommandResult AssignShader_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiAssignShader(args[1].id, args[2].id);
  return result;
}

/* AssignVolume */
static const int AssignVolume_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static struct CommandResult AssignVolume_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiAssignVolume(args[1].id, args[2].str, args[3].id);
  return result;
}

/* AssignMesh */
static const int AssignMesh_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static struct CommandResult AssignMesh_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiAssignMesh(args[1].id, args[2].str, args[3].id);
  return result;
}

/* SetProperty1 */
static const int SetProperty1_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_NUMBER};
static struct CommandResult SetProperty1_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiSetProperty1(args[1].id, args[2].str, args[3].num);
  return result;
}

/* SetProperty2 */
static const int SetProperty2_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_NUMBER,
  ARG_NUMBER};
static struct CommandResult SetProperty2_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiSetProperty2(
      args[1].id, args[2].str, args[3].num, args[4].num);
  return result;
}

/* SetProperty3 */
static const int SetProperty3_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_NUMBER,
  ARG_NUMBER,
  ARG_NUMBER};
static struct CommandResult SetProperty3_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiSetProperty3(
      args[1].id, args[2].str,
      args[3].num, args[4].num, args[5].num);
  return result;
}

/* SetProperty4 */
static const int SetProperty4_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_NUMBER,
  ARG_NUMBER,
  ARG_NUMBER,
  ARG_NUMBER};
static struct CommandResult SetProperty4_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiSetProperty4(
      args[1].id, args[2].str,
      args[3].num, args[4].num, args[5].num, args[6].num);
  return result;
}

/* SetStringProperty */
static const int SetStringProperty_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_STRING};
static struct CommandResult SetStringProperty_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiSetStringProperty(args[1].id, args[2].str, args[3].str);
  return result;
}

/* SetSampleProperty3 */
static const int SetSampleProperty3_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_NUMBER,
  ARG_NUMBER,
  ARG_NUMBER,
  ARG_NUMBER};
static struct CommandResult SetSampleProperty3_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = SiSetSampleProperty3(
      args[1].id, args[2].str,
      args[3].num, args[4].num, args[5].num, args[6].num);
  return result;
}

/* ShowPropertyList */
static const int ShowPropertyList_args[] = {
  ARG_COMMAND_NAME,
  ARG_STRING};
static struct CommandResult ShowPropertyList_run(const struct CommandArgument *args)
{
  struct CommandResult result = INIT_COMMAND_RESULT;
  result.status = print_property_list(args[1].str);
  return result;
}

static const struct Command command_list[] = {
#define REGISTER_COMMAND(name) {#name, name##_args, \
    sizeof(name##_args)/sizeof(name##_args[0]), name##_run}
  REGISTER_COMMAND(OpenPlugin),
  REGISTER_COMMAND(RenderScene),
  REGISTER_COMMAND(RunProcedure),
  REGISTER_COMMAND(SaveFrameBuffer),
  REGISTER_COMMAND(AddObjectToGroup),
  REGISTER_COMMAND(NewObjectInstance),
  REGISTER_COMMAND(NewFrameBuffer),
  REGISTER_COMMAND(NewObjectGroup),
  REGISTER_COMMAND(NewPointCloud),
  REGISTER_COMMAND(NewTurbulence),
  REGISTER_COMMAND(NewProcedure),
  REGISTER_COMMAND(NewRenderer),
  REGISTER_COMMAND(NewTexture),
  REGISTER_COMMAND(NewShader),
  REGISTER_COMMAND(NewCamera),
  REGISTER_COMMAND(NewVolume),
  REGISTER_COMMAND(NewCurve),
  REGISTER_COMMAND(NewLight),
  REGISTER_COMMAND(NewMesh),
  REGISTER_COMMAND(AssignFrameBuffer),
  REGISTER_COMMAND(AssignObjectGroup),
  REGISTER_COMMAND(AssignTurbulence),
  REGISTER_COMMAND(AssignTexture),
  REGISTER_COMMAND(AssignCamera),
  REGISTER_COMMAND(AssignShader),
  REGISTER_COMMAND(AssignVolume),
  REGISTER_COMMAND(AssignMesh),
  REGISTER_COMMAND(SetProperty1),
  REGISTER_COMMAND(SetProperty2),
  REGISTER_COMMAND(SetProperty3),
  REGISTER_COMMAND(SetProperty4),
  REGISTER_COMMAND(SetStringProperty),
  REGISTER_COMMAND(SetSampleProperty3),
  REGISTER_COMMAND(ShowPropertyList),
  {NULL, NULL, 0, NULL}
#undef REGISTER_COMMAND
};

const struct Command *CmdSearchCommand(const char *command_name)
{
  const struct Command *cmd = command_list;

  for (; cmd->name != NULL; cmd++) {
    if (strcmp(cmd->name, command_name) == 0) {
      return cmd;
    }
  }

  return NULL;
}

int CmdSuccess(const struct CommandResult *result)
{
  if (result->status == SI_FAIL &&
      result->new_entry_id == SI_BADID) {
    return 0;
  } else {
    return 1;
  }
}

static int print_property_list(const char *type_name)
{
  const char **prop_types = NULL;
  const char **prop_names = NULL;
  int nprops = 0;
  int err = 0;
  int i;

  err = SiGetPropertyList(type_name, &prop_types, &prop_names, &nprops);
  if (err) {
    printf("#   No property is available for %s\n", type_name);
    printf("#   Make sure the type name is correct.\n");
    printf("#   If you requested properties for a plugin,\n");
    printf("#   make sure it is opened before calling this command.\n");
    return -1;
  }

  printf("#   %s Properties\n", type_name);
  printf("#   %15.15s   %-20.20s\n", "(Type)", "(Name)");
  for (i = 0; i < nprops; i++) {
    printf("#   %15.15s : %-20.20s\n", prop_types[i], prop_names[i]);
  }

  return 0;
}

