// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "command.h"
#include "fj_transform.h"
#include "fj_property.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int print_property_list(const char *type_name);

#if TESTTEST
// OpenPlugin
class OpenPluginCommand : public Command {
  OpenPluginCommand()
  {
    SetArguments(
      ARG_COMMAND_NAME,
      ARG_NEW_ENTRY_ID,
      ARG_FILE_PATH);
  }
  ~OpenPluginCommand() {}
  CommandResult Run(const std::vector<CommandArgument> &args) const
  {
    CommandResult result;
    result.SetEntryID(SiOpenPlugin(args[2].GetString()));
    result.SetEntryName(args[1].GetString());
    return result;
  }
}
#endif

/* OpenPlugin */
static const int OpenPlugin_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_FILE_PATH};
static CommandResult OpenPlugin_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiOpenPlugin(args[2].GetString()));
  result.SetEntryName(args[1].GetString());
  return result;
}

/* RenderScene */
static const int RenderScene_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID};
static CommandResult RenderScene_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiRenderScene(args[1].GetID()));
  return result;
}

/* RunProcedure */
static const int RunProcedure_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID};
static CommandResult RunProcedure_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiRunProcedure(args[1].GetID()));
  return result;
}

/* SaveFrameBuffer */
static const int SaveFrameBuffer_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_FILE_PATH};
static CommandResult SaveFrameBuffer_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiSaveFrameBuffer(args[1].GetID(), args[2].GetString()));
  return result;
}

/* AddObjectToGroup */
static const int AddObjectToGroup_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_ENTRY_ID};
static CommandResult AddObjectToGroup_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAddObjectToGroup(args[1].GetID(), args[2].GetID()));
  return result;
}

/* NewObjectInstance */
static const int NewObjectInstance_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_ENTRY_ID};
static CommandResult NewObjectInstance_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewObjectInstance(args[2].GetID()));
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewFrameBuffer */
static const int NewFrameBuffer_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_STRING};
static CommandResult NewFrameBuffer_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewFrameBuffer(args[2].GetString()));
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewObjectGroup */
static const int NewObjectGroup_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static CommandResult NewObjectGroup_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewObjectGroup());
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewPointCloud */
static const int NewPointCloud_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static CommandResult NewPointCloud_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewPointCloud());
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewTurbulence */
static const int NewTurbulence_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static CommandResult NewTurbulence_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewTurbulence());
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewProcedure */
static const int NewProcedure_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_ENTRY_ID};
static CommandResult NewProcedure_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewProcedure(args[2].GetID()));
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewRenderer */
static const int NewRenderer_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static CommandResult NewRenderer_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewRenderer());
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewTexture */
static const int NewTexture_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_FILE_PATH};
static CommandResult NewTexture_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewTexture(args[2].GetString()));
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewShader */
static const int NewShader_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_ENTRY_ID};
static CommandResult NewShader_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewShader(args[2].GetID()));
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewCamera */
static const int NewCamera_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_STRING};
static CommandResult NewCamera_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewCamera(args[2].GetString()));
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewVolume */
static const int NewVolume_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static CommandResult NewVolume_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewVolume());
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewCurve */
static const int NewCurve_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static CommandResult NewCurve_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewCurve());
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewLight */
static const int NewLight_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID,
  ARG_LIGHT_TYPE};
static CommandResult NewLight_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewLight(args[2].GetNumber()));
  result.SetEntryName(args[1].GetString());
  return result;
}

/* NewMesh */
static const int NewMesh_args[] = {
  ARG_COMMAND_NAME,
  ARG_NEW_ENTRY_ID};
static CommandResult NewMesh_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetEntryID(SiNewMesh());
  result.SetEntryName(args[1].GetString());
  return result;
}

/* AssignFrameBuffer */
static const int AssignFrameBuffer_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_ENTRY_ID};
static CommandResult AssignFrameBuffer_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAssignFrameBuffer(args[1].GetID(), args[2].GetID()));
  return result;
}

/* AssignObjectGroup */
static const int AssignObjectGroup_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static CommandResult AssignObjectGroup_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAssignObjectGroup(args[1].GetID(), args[2].GetString(), args[3].GetID()));
  return result;
}

/* AssignPointCloud */
static const int AssignPointCloud_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static CommandResult AssignPointCloud_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAssignPointCloud(args[1].GetID(), args[2].GetString(), args[3].GetID()));
  return result;
}

/* AssignTurbulence */
static const int AssignTurbulence_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static CommandResult AssignTurbulence_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAssignTurbulence(args[1].GetID(), args[2].GetString(), args[3].GetID()));
  return result;
}

/* AssignTexture */
static const int AssignTexture_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static CommandResult AssignTexture_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAssignTexture(args[1].GetID(), args[2].GetString(), args[3].GetID()));
  return result;
}

/* AssignCamera */
static const int AssignCamera_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_ENTRY_ID};
static CommandResult AssignCamera_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAssignCamera(args[1].GetID(), args[2].GetID()));
  return result;
}

/* AssignShader */
static const int AssignShader_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_GROUP_NAME,
  ARG_ENTRY_ID};
static CommandResult AssignShader_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAssignShader(args[1].GetID(), args[2].GetString(), args[3].GetID()));
  return result;
}

/* AssignVolume */
static const int AssignVolume_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static CommandResult AssignVolume_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAssignVolume(args[1].GetID(), args[2].GetString(), args[3].GetID()));
  return result;
}

/* AssignCurve */
static const int AssignCurve_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static CommandResult AssignCurve_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAssignCurve(args[1].GetID(), args[2].GetString(), args[3].GetID()));
  return result;
}

/* AssignMesh */
static const int AssignMesh_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_ENTRY_ID};
static CommandResult AssignMesh_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiAssignMesh(args[1].GetID(), args[2].GetString(), args[3].GetID()));
  return result;
}

/* SetProperty1 */
static const int SetProperty1_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_NUMBER};
static CommandResult SetProperty1_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiSetProperty1(args[1].GetID(), args[2].GetString(), args[3].GetNumber()));
  return result;
}

/* SetProperty2 */
static const int SetProperty2_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_NUMBER,
  ARG_NUMBER};
static CommandResult SetProperty2_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiSetProperty2(
      args[1].GetID(), args[2].GetString(), args[3].GetNumber(), args[4].GetNumber()));
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
static CommandResult SetProperty3_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiSetProperty3(
      args[1].GetID(), args[2].GetString(),
      args[3].GetNumber(), args[4].GetNumber(), args[5].GetNumber()));
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
static CommandResult SetProperty4_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiSetProperty4(
      args[1].GetID(), args[2].GetString(),
      args[3].GetNumber(), args[4].GetNumber(), args[5].GetNumber(), args[6].GetNumber()));
  return result;
}

/* SetStringProperty */
static const int SetStringProperty_args[] = {
  ARG_COMMAND_NAME,
  ARG_ENTRY_ID,
  ARG_PROPERTY_NAME,
  ARG_STRING};
static CommandResult SetStringProperty_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiSetStringProperty(args[1].GetID(), args[2].GetString(), args[3].GetString()));
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
static CommandResult SetSampleProperty3_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(SiSetSampleProperty3(
      args[1].GetID(), args[2].GetString(),
      args[3].GetNumber(), args[4].GetNumber(), args[5].GetNumber(), args[6].GetNumber()));
  return result;
}

/* ShowPropertyList */
static const int ShowPropertyList_args[] = {
  ARG_COMMAND_NAME,
  ARG_STRING};
static CommandResult ShowPropertyList_run(const CommandArgument *args)
{
  CommandResult result;
  result.SetStatus(print_property_list(args[1].GetString()));
  return result;
}

static const Command command_list[] = {
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
  REGISTER_COMMAND(AssignPointCloud),
  REGISTER_COMMAND(AssignTurbulence),
  REGISTER_COMMAND(AssignTexture),
  REGISTER_COMMAND(AssignCamera),
  REGISTER_COMMAND(AssignShader),
  REGISTER_COMMAND(AssignVolume),
  REGISTER_COMMAND(AssignCurve),
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

const Command *SearchCommand(const char *command_name)
{
  const Command *cmd = command_list;

  for (; cmd->name != NULL; cmd++) {
    if (strcmp(cmd->name, command_name) == 0) {
      return cmd;
    }
  }

  return NULL;
}

static void scalar_to_transform_order_string(char *dst, const char *prop_name, double value)
{
  if (strcmp(prop_name, "transform_order") == 0 || strcmp(prop_name, "rotate_order") == 0) {
    const int order = (int) value;
    const char *order_str = NULL;

    switch (order) {
    case ORDER_SRT: order_str = "ORDER_SRT"; break;
    case ORDER_STR: order_str = "ORDER_STR"; break;
    case ORDER_RST: order_str = "ORDER_RST"; break;
    case ORDER_RTS: order_str = "ORDER_RTS"; break;
    case ORDER_TRS: order_str = "ORDER_TRS"; break;
    case ORDER_TSR: order_str = "ORDER_TSR"; break;
    case ORDER_XYZ: order_str = "ORDER_XYZ"; break;
    case ORDER_XZY: order_str = "ORDER_XZY"; break;
    case ORDER_YXZ: order_str = "ORDER_YXZ"; break;
    case ORDER_YZX: order_str = "ORDER_YZX"; break;
    case ORDER_ZXY: order_str = "ORDER_ZXY"; break;
    case ORDER_ZYX: order_str = "ORDER_ZYX"; break;
    }
    sprintf(dst, "(%s)", order_str);
    return;
  }

  sprintf(dst, "%g", value);
}

static int print_property_list(const char *type_name)
{
  const Property *list = SiGetPropertyList(type_name);
  const Property *prop = NULL;

  if (list == NULL) {
    printf("#   No property is available for %s\n", type_name);
    printf("#   Make sure the type name is correct.\n");
    printf("#   If you requested properties for a plugin,\n");
    printf("#   make sure it is opened before calling this command.\n");
    return -1;
  }

  printf("#   %s Properties\n", type_name);
  printf("#   %-15.15s   %-20.20s   %-20.20s\n", "(Type)", "(Name)", "(Default)");
  for (prop = list; prop->IsValid(); prop++) {
    double default_value[4] = {0, 0, 0, 0};
    char default_value_string[512] = {'\0'};

    default_value[0] = prop->GetDefaultValue()[0];
    default_value[1] = prop->GetDefaultValue()[1];
    default_value[2] = prop->GetDefaultValue()[2];
    default_value[3] = prop->GetDefaultValue()[3];

    switch (prop->GetType()) {
      case PROP_SCALAR:
        scalar_to_transform_order_string(
            default_value_string,
            prop->GetName(),
            default_value[0]);
        break;
      case PROP_VECTOR2:
        sprintf(default_value_string, "(%g, %g)",
            default_value[0],
            default_value[1]);
        break;
      case PROP_VECTOR3:
        sprintf(default_value_string, "(%g, %g, %g)",
            default_value[0],
            default_value[1],
            default_value[2]);
        break;
      case PROP_VECTOR4:
        sprintf(default_value_string, "(%g, %g, %g, %g)",
            default_value[0],
            default_value[1],
            default_value[2],
            default_value[3]);
        break;
      default:
        sprintf(default_value_string, "(null)");
        break;
    }

    printf("#   %-15.15s : %-20.20s : %-20.20s\n",
        prop->GetTypeString(),
        prop->GetName(),
        default_value_string);
  }

  return 0;
}

CommandArgument::CommandArgument() :
  str_("N/A"), num_(0), id_(SI_BADID) //TODO string for invalid argument
{
}

CommandArgument::~CommandArgument()
{
}

void CommandArgument::SetString(const std::string &str)
{
  str_ = str;
}

const char *CommandArgument::GetString() const
{
  return str_.c_str();
}

void CommandArgument::SetNumber(double num)
{
  num_ = num;
}

double CommandArgument::GetNumber() const
{
  return num_;
}

void CommandArgument::SetID(ID id)
{
  id_ = id;
}

ID CommandArgument::GetID() const
{
  return id_;
}

CommandResult::CommandResult() :
    status_(SI_FAIL), new_id_(SI_BADID), entry_name_("")
{
}

CommandResult::~CommandResult()
{
}

void CommandResult::SetStatus(Status status)
{
  status_ = status;
}

Status CommandResult::GetStatus() const
{
  return status_;
}

void CommandResult::SetEntryID(ID id)
{
  new_id_ = id;
}

ID CommandResult::GetEntryID() const
{
  return new_id_;
}

void CommandResult::SetEntryName(const std::string &name)
{
  entry_name_ = name;
}

const char *CommandResult::GetEntryName() const
{
  return entry_name_.c_str();
}

bool CommandResult::HasEntryName() const
{
  return !entry_name_.empty();
}

bool CommandResult::IsFail() const
{
  return GetStatus() == SI_FAIL && GetEntryID() == SI_BADID;
}
