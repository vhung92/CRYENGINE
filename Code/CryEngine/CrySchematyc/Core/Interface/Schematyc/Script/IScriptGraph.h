// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "Schematyc/Script/IScriptElement.h"
#include "Schematyc/Script/IScriptExtension.h"
#include "Schematyc/Script/ScriptDependencyEnumerator.h"
#include "Schematyc/Utils/EnumFlags.h"
#include "Schematyc/Utils/GraphPortId.h"
#include "Schematyc/Utils/GUID.h"
#include "Schematyc/Utils/Signal.h"
#include "Schematyc/Utils/Validator.h"

namespace Schematyc
{
// Forward declare interfaces.
struct IGraphNodeCompiler;
struct IScriptGraph;
struct IScriptGraphNodeCreationMenu;
// Forward declare structures.
struct SCompilerContext;
struct SElementId;
// Forward declare classes.
class CAnyConstPtr;

enum class EScriptGraphColor
{
	NotSet = 0,
	Red,
	Green,
	Blue,
	Yellow,
	Orange,
	Purple
};

enum class EScriptGraphNodeFlags
{
	NotCopyable  = BIT(0),
	NotRemovable = BIT(1)
};

typedef CEnumFlags<EScriptGraphNodeFlags> ScriptGraphNodeFlags;

enum class EScriptGraphPortFlags
{
	None = 0,

	// Category.
	Signal = BIT(0),        // Port is signal.
	Flow   = BIT(1),        // Port controls internal flow.
	Data   = BIT(2),        // Port has data.

	// Connection modifiers.
	MultiLink = BIT(3),     // Multiple links can be connected to port.

	// Flow modifiers.
	Begin = BIT(4),         // Port initiates flow.
	End   = BIT(5),         // Port terminates flow.

	// Data modifiers.
	Ptr        = BIT(6),  // Data is pointer.
	Array      = BIT(7),  // Data is array.
	Persistent = BIT(8),  // Data will be saved to file.
	Editable   = BIT(9),  // Data can be modified in editor.
	Pull       = BIT(10), // Data can be pulled from node without need to trigger an input.

	// Layout modifiers.
	SpacerAbove = BIT(11),   // Draw spacer above port.
	SpacerBelow = BIT(12)    // Draw spacer below port.
};

typedef CEnumFlags<EScriptGraphPortFlags> ScriptGraphPortFlags;

struct IScriptGraphNode // #SchematycTODO : Move to separate header?
{
	virtual ~IScriptGraphNode() {}

	virtual void                 Attach(IScriptGraph& graph) = 0;
	virtual IScriptGraph&        GetGraph() = 0;
	virtual const IScriptGraph&  GetGraph() const = 0;
	virtual SGUID                GetTypeGUID() const = 0;
	virtual SGUID                GetGUID() const = 0;
	virtual const char*          GetName() const = 0;
	virtual ColorB               GetColor() const = 0;
	virtual ScriptGraphNodeFlags GetFlags() const = 0;
	virtual void                 SetPos(Vec2 pos) = 0;
	virtual Vec2                 GetPos() const = 0;
	virtual uint32               GetInputCount() const = 0;
	virtual uint32               FindInputById(const CGraphPortId& id) const = 0;
	virtual CGraphPortId         GetInputId(uint32 inputIdx) const = 0;
	virtual const char*          GetInputName(uint32 inputIdx) const = 0;
	virtual SGUID                GetInputTypeGUID(uint32 inputIdx) const = 0;
	virtual ScriptGraphPortFlags GetInputFlags(uint32 inputIdx) const = 0;
	virtual CAnyConstPtr         GetInputData(uint32 inputIdx) const = 0;
	virtual ColorB               GetInputColor(uint32 inputIdx) const = 0;
	virtual uint32               GetOutputCount() const = 0;
	virtual uint32               FindOutputById(const CGraphPortId& id) const = 0;
	virtual CGraphPortId         GetOutputId(uint32 outputIdx) const = 0;
	virtual const char*          GetOutputName(uint32 outputIdx) const = 0;
	virtual SGUID                GetOutputTypeGUID(uint32 outputIdx) const = 0;
	virtual ScriptGraphPortFlags GetOutputFlags(uint32 outputIdx) const = 0;
	virtual CAnyConstPtr         GetOutputData(uint32 outputIdx) const = 0;
	virtual ColorB               GetOutputColor(uint32 outputIdx) const = 0;
	virtual void                 EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const = 0;
	virtual void                 ProcessEvent(const SScriptEvent& event) = 0;
	virtual void                 Serialize(Serialization::IArchive& archive) = 0;
	virtual void                 Copy(Serialization::IArchive& archive) = 0; // #SchematycTODO : Rather than having an explicit copy function consider using standard save pass with a copy flag set in the context.
	virtual void                 Paste(Serialization::IArchive& archive) = 0;  // #SchematycTODO : Rather than having an explicit paste function consider using standard load passes with a paste flag set in the context.
	virtual void                 Validate(const Validator& validator) const = 0;
	virtual void                 RemapDependencies(IGUIDRemapper& guidRemapper) = 0;
	virtual void                 Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const {}
};

DECLARE_SHARED_POINTERS(IScriptGraphNode)

typedef CDelegate<EVisitStatus(IScriptGraphNode&)>       ScriptGraphNodeVisitor;
typedef CDelegate<EVisitStatus(const IScriptGraphNode&)> ScriptGraphNodeConstVisitor;

struct IScriptGraphLink // #SchematycTODO : Once all ports are referenced by id we can replace this with a simple SSCriptGraphLink structure.
{
	virtual ~IScriptGraphLink() {}

	virtual void         SetSrcNodeGUID(const SGUID& guid) = 0;
	virtual SGUID        GetSrcNodeGUID() const = 0;
	virtual CGraphPortId GetSrcOutputId() const = 0;
	virtual void         SetDstNodeGUID(const SGUID& guid) = 0;
	virtual SGUID        GetDstNodeGUID() const = 0;
	virtual CGraphPortId GetDstInputId() const = 0;
	virtual void         Serialize(Serialization::IArchive& archive) = 0;
};

DECLARE_SHARED_POINTERS(IScriptGraphLink)

typedef CDelegate<EVisitStatus(IScriptGraphLink&)>       ScriptGraphLinkVisitor;
typedef CDelegate<EVisitStatus(const IScriptGraphLink&)> ScriptGraphLinkConstVisitor;

enum class EScriptGraphType // #SchematycTODO : Do we really need this enumeration or can we get the information we need from the element that owns the graph?
{
	Construction,
	Signal,
	Function,
	Transition
};

struct SScriptGraphParams
{
	inline SScriptGraphParams(const SGUID& _scopeGUID, const char* _szName, EScriptGraphType _type, const SGUID& _contextGUID)
		: scopeGUID(_scopeGUID)
		, szName(_szName)
		, type(_type)
		, contextGUID(_contextGUID)
	{}

	const SGUID&     scopeGUID;
	const char*      szName;
	EScriptGraphType type;
	const SGUID&     contextGUID;
};

typedef CSignal<void (const IScriptGraphLink&)> ScriptGraphLinkRemovedSignal;

struct IScriptGraphNodeCreationMenuCommand
{
	virtual ~IScriptGraphNodeCreationMenuCommand() {}

	virtual IScriptGraphNodePtr Execute(const Vec2& pos) = 0;
};

DECLARE_SHARED_POINTERS(IScriptGraphNodeCreationMenuCommand)

struct IScriptGraphNodeCreationMenu
{
	virtual ~IScriptGraphNodeCreationMenu() {}

	virtual bool AddOption(const char* szLabel, const char* szDescription, const char* szWikiLink, const IScriptGraphNodeCreationMenuCommandPtr& pCommand) = 0;
};

struct IScriptGraph : public IScriptExtensionBase<EScriptExtensionType::Graph>
{
	virtual ~IScriptGraph() {}

	virtual EScriptGraphType                     GetType() const = 0;

	virtual void                                 SetPos(Vec2 pos) = 0;
	virtual Vec2                                 GetPos() const = 0;

	virtual void                                 PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu) = 0;
	virtual bool                                 AddNode(const IScriptGraphNodePtr& pNode) = 0;
	virtual IScriptGraphNodePtr                  AddNode(const SGUID& typeGUID) = 0;
	virtual void                                 RemoveNode(const SGUID& guid) = 0;
	virtual uint32                               GetNodeCount() const = 0;
	virtual IScriptGraphNode*                    GetNode(const SGUID& guid) = 0;
	virtual const IScriptGraphNode*              GetNode(const SGUID& guid) const = 0;
	virtual void                                 VisitNodes(const ScriptGraphNodeVisitor& visitor) = 0;
	virtual void                                 VisitNodes(const ScriptGraphNodeConstVisitor& visitor) const = 0;

	virtual bool                                 CanAddLink(const SGUID& srcNodeGUID, const CGraphPortId& srcOutputId, const SGUID& dstNodeGUID, const CGraphPortId& dstInputId) const = 0;
	virtual IScriptGraphLink*                    AddLink(const SGUID& srcNodeGUID, const CGraphPortId& srcOutputId, const SGUID& dstNodeGUID, const CGraphPortId& dstInputId) = 0;
	virtual void                                 RemoveLink(uint32 iLink) = 0;
	virtual void                                 RemoveLinks(const SGUID& nodeGUID) = 0;
	virtual uint32                               GetLinkCount() const = 0;
	virtual IScriptGraphLink*                    GetLink(uint32 linkIdx) = 0;
	virtual const IScriptGraphLink*              GetLink(uint32 linkIdx) const = 0;
	virtual uint32                               FindLink(const SGUID& srcNodeGUID, const CGraphPortId& srcOutputId, const SGUID& dstNodeGUID, const CGraphPortId& dstInputId) const = 0;
	virtual EVisitResult                         VisitLinks(const ScriptGraphLinkVisitor& visitor) = 0;
	virtual EVisitResult                         VisitLinks(const ScriptGraphLinkConstVisitor& visitor) const = 0;
	virtual EVisitResult                         VisitInputLinks(const ScriptGraphLinkVisitor& visitor, const SGUID& dstNodeGUID, const CGraphPortId& dstInputId) = 0;
	virtual EVisitResult                         VisitInputLinks(const ScriptGraphLinkConstVisitor& visitor, const SGUID& dstNodeGUID, const CGraphPortId& dstInputId) const = 0;
	virtual EVisitResult                         VisitOutputLinks(const ScriptGraphLinkVisitor& visitor, const SGUID& srcNodeGUID, const CGraphPortId& srcOutputId) = 0;
	virtual EVisitResult                         VisitOutputLinks(const ScriptGraphLinkConstVisitor& visitor, const SGUID& srcNodeGUID, const CGraphPortId& srcOutputId) const = 0;
	virtual bool                                 GetLinkSrc(const IScriptGraphLink& link, IScriptGraphNode*& pNode, uint32& outputIdx) = 0;
	virtual bool                                 GetLinkSrc(const IScriptGraphLink& link, const IScriptGraphNode*& pNode, uint32& outputIdx) const = 0;
	virtual bool                                 GetLinkDst(const IScriptGraphLink& link, IScriptGraphNode*& pNode, uint32& inputIdx) = 0;
	virtual bool                                 GetLinkDst(const IScriptGraphLink& link, const IScriptGraphNode*& pNode, uint32& inputIdx) const = 0;

	virtual void                                 RemoveBrokenLinks() = 0;
	virtual ScriptGraphLinkRemovedSignal::Slots& GetLinkRemovedSignalSlots() = 0;
};
} // Schematyc