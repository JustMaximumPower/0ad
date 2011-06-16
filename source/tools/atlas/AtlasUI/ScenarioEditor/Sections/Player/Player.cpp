/* Copyright (C) 2011 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "precompiled.h"

#include "Player.h"

#include "AtlasObject/AtlasObject.h"
#include "AtlasScript/ScriptInterface.h"
#include "ScenarioEditor/ScenarioEditor.h"

#include "wx/choicebk.h"

enum
{
	ID_NumPlayers,
	ID_PlayerFood,
	ID_PlayerWood,
	ID_PlayerMetal,
	ID_PlayerStone,
	ID_PlayerPop,
	ID_PlayerColour,
	ID_PlayerHuman,
	ID_PlayerAI,
	ID_CameraSet,
	ID_CameraView,
	ID_CameraClear
};

// TODO: Some of these helper things should be moved out of this file
// and into shared locations

// Helper function for adding tooltips
static wxWindow* Tooltipped(wxWindow* window, const wxString& tip)
{
	window->SetToolTip(tip);
	return window;
}

//////////////////////////////////////////////////////////////////////////

class PlayerNotebookPage : public wxPanel
{

public:
	PlayerNotebookPage(ScenarioEditor& scenarioEditor, wxWindow* parent, const wxString& name, size_t playerID)
		: wxPanel(parent, wxID_ANY), m_ScenarioEditor(scenarioEditor), m_Name(name), m_PlayerID(playerID)
	{

		m_Controls.page = this;

		Freeze();

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(sizer);

		/////////////////////////////////////////////////////////////////////////
		// Player Info
		wxStaticBoxSizer* playerInfoSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Player info"));
		wxFlexGridSizer* gridSizer = new wxFlexGridSizer(2, 2, 5, 5);
		gridSizer->AddGrowableCol(1);
		gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Name")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		wxTextCtrl* nameCtrl = new wxTextCtrl(this, wxID_ANY);
		gridSizer->Add(nameCtrl, wxSizerFlags(1).Expand().Align(wxALIGN_RIGHT));
		m_Controls.name = nameCtrl;
		gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Civilisation")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		wxChoice* civChoice = new wxChoice(this, wxID_ANY);
		gridSizer->Add(civChoice, wxSizerFlags(1).Expand().Align(wxALIGN_RIGHT));
		m_Controls.civ = civChoice;
		gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Colour")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		wxButton* colourButton = new wxButton(this, ID_PlayerColour);
		gridSizer->Add(Tooltipped(colourButton, _("Set player colour")), wxSizerFlags(1).Expand().Align(wxALIGN_RIGHT));
		m_Controls.colour = colourButton;
		gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Default AI")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		wxChoice* aiChoice = new wxChoice(this, wxID_ANY);
		gridSizer->Add(Tooltipped(aiChoice, _("Select default AI")), wxSizerFlags(1).Expand().Align(wxALIGN_RIGHT));
		m_Controls.ai = aiChoice;

		playerInfoSizer->Add(gridSizer, wxSizerFlags(1).Expand());
		sizer->Add(playerInfoSizer, wxSizerFlags().Expand());

		/////////////////////////////////////////////////////////////////////////
		// Resources
		wxStaticBoxSizer* resourceSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Resources"));
		gridSizer = new wxFlexGridSizer(2, 2, 5, 5);
		gridSizer->AddGrowableCol(1);
		gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Food")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		wxSpinCtrl* foodCtrl = new wxSpinCtrl(this, ID_PlayerFood, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, INT_MAX);
		gridSizer->Add(Tooltipped(foodCtrl, _("Initial value of food resource")));
		m_Controls.food = foodCtrl;
		gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Wood")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		wxSpinCtrl* woodCtrl = new wxSpinCtrl(this, ID_PlayerWood, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, INT_MAX);
		gridSizer->Add(Tooltipped(woodCtrl, _("Initial value of wood resource")));
		m_Controls.wood = woodCtrl;
		gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Metal")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		wxSpinCtrl* metalCtrl = new wxSpinCtrl(this, ID_PlayerMetal, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, INT_MAX);
		gridSizer->Add(Tooltipped(metalCtrl, _("Initial value of metal resource")));
		m_Controls.metal = metalCtrl;
		gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Stone")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		wxSpinCtrl* stoneCtrl = new wxSpinCtrl(this, ID_PlayerStone, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, INT_MAX);
		gridSizer->Add(Tooltipped(stoneCtrl, _("Initial value of stone resource")));
		m_Controls.stone = stoneCtrl;
		gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Pop limit")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		wxSpinCtrl* popCtrl = new wxSpinCtrl(this, ID_PlayerPop, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, INT_MAX);
		gridSizer->Add(Tooltipped(popCtrl, _("Population limit for this player")));
		m_Controls.pop = popCtrl;

		resourceSizer->Add(gridSizer, wxSizerFlags(1).Expand());
		sizer->Add(resourceSizer, wxSizerFlags().Expand());

		/////////////////////////////////////////////////////////////////////////
		// Diplomacy
		wxStaticBoxSizer* diplomacySizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Diplomacy"));
		wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
		boxSizer->Add(new wxStaticText(this, wxID_ANY, _("Team")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		wxChoice* teamCtrl = new wxChoice(this, wxID_ANY);
		teamCtrl->Append(_("None"));
		teamCtrl->Append(_T("1"));
		teamCtrl->Append(_T("2"));
		teamCtrl->Append(_T("3"));
		teamCtrl->Append(_T("4"));
		boxSizer->Add(teamCtrl);
		m_Controls.team = teamCtrl;
		diplomacySizer->Add(boxSizer, wxSizerFlags(1).Expand());

		// TODO: possibly have advanced panel where each player's diplomacy can be set?
		// Advanced panel
		/*wxCollapsiblePane* advPane = new wxCollapsiblePane(this, wxID_ANY, _("Advanced"));
		wxWindow* pane = advPane->GetPane();
		diplomacySizer->Add(advPane, 0, wxGROW | wxALL, 2);*/

		sizer->Add(diplomacySizer, wxSizerFlags().Expand());

		/////////////////////////////////////////////////////////////////////////
		// Camera
		wxStaticBoxSizer* cameraSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Starting Camera"));
		wxButton* cameraSet = new wxButton(this, ID_CameraSet, _("Set"));
		cameraSizer->Add(Tooltipped(cameraSet, _("Set player camera to this view")), wxSizerFlags(1));
		wxButton* cameraView = new wxButton(this, ID_CameraView, _("View"));
		cameraView->Enable(false);
		cameraSizer->Add(Tooltipped(cameraView, _("View the player camera")), wxSizerFlags(1));
		wxButton* cameraClear = new wxButton(this, ID_CameraClear, _("Clear"));
		cameraClear->Enable(false);
		cameraSizer->Add(Tooltipped(cameraClear, _("Clear player camera")), wxSizerFlags(1));

		sizer->Add(cameraSizer, wxSizerFlags().Expand());

		Layout();
		Thaw();

	}

	void OnDisplay()
	{
	}

	PlayerPageControls GetControls()
	{
		return m_Controls;
	}

	wxString GetPlayerName()
	{
		return m_Name;
	}

	size_t GetPlayerID()
	{
		return m_PlayerID;
	}

	bool IsCameraDefined()
	{
		return m_CameraDefined;
	}

	sCameraInfo GetCamera()
	{
		return m_Camera;
	}

	void SetCamera(sCameraInfo info, bool isDefined = true)
	{
		m_Camera = info;
		m_CameraDefined = isDefined;

		// Enable/disable controls
		wxDynamicCast(FindWindow(ID_CameraView), wxButton)->Enable(isDefined);
		wxDynamicCast(FindWindow(ID_CameraClear), wxButton)->Enable(isDefined);
	}

private:
	void OnColour(wxCommandEvent& evt)
	{
		// Show colour dialog
		wxColourData clrData;
		clrData.SetColour(m_Controls.colour->GetBackgroundColour());

		wxColourDialog colourDlg(this, &clrData);
		colourDlg.SetTitle(_("Choose player colour"));

		if (colourDlg.ShowModal() == wxID_OK)
		{
			m_Controls.colour->SetBackgroundColour(colourDlg.GetColourData().GetColour());
			
			// Pass event on to next handler
			evt.Skip();
		}
	}

	void OnCameraSet(wxCommandEvent& evt)
	{
		AtlasMessage::qGetView qryView;
		qryView.Post();
		SetCamera(qryView.info, true);

		// Pass event on to next handler
		evt.Skip();
	}

	void OnCameraView(wxCommandEvent& WXUNUSED(evt))
	{
		POST_MESSAGE(SetView, (m_Camera));
	}

	void OnCameraClear(wxCommandEvent& evt)
	{
		SetCamera(sCameraInfo(), false);

		// Pass event on to next handler
		evt.Skip();
	}

	sCameraInfo m_Camera;
	bool m_CameraDefined;
	wxString m_Name;
	size_t m_PlayerID;
	ScenarioEditor& m_ScenarioEditor;
	
	PlayerPageControls m_Controls;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(PlayerNotebookPage, wxPanel)
	EVT_BUTTON(ID_PlayerColour, PlayerNotebookPage::OnColour)
	EVT_BUTTON(ID_CameraSet, PlayerNotebookPage::OnCameraSet)
	EVT_BUTTON(ID_CameraView, PlayerNotebookPage::OnCameraView)
	EVT_BUTTON(ID_CameraClear, PlayerNotebookPage::OnCameraClear)
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////////////

class PlayerNotebook : public wxChoicebook
{
public:
	PlayerNotebook(ScenarioEditor& scenarioEditor, wxWindow *parent)
		: wxChoicebook(parent, wxID_ANY/*, wxDefaultPosition, wxDefaultSize, wxNB_FIXEDWIDTH*/),
		  m_ScenarioEditor(scenarioEditor)
	{
	}

	PlayerPageControls AddPlayer(wxString name, size_t player)
	{
		PlayerNotebookPage* playerPage = new PlayerNotebookPage(m_ScenarioEditor, this, name, player);
		AddPage(playerPage, name);
		m_Pages.push_back(playerPage);
		return playerPage->GetControls();
	}

	void ResizePlayers(size_t numPlayers)
	{
		// We don't really want to destroy the windows corresponding
		//		to the tabs, so we've kept them in a vector and will
		//		only remove and add them to the notebook as needed
		if (numPlayers <= m_Pages.size())
		{
			size_t pageCount = GetPageCount();
			if (numPlayers > pageCount)
			{
				// Add previously removed pages
				for (size_t i = pageCount; i < numPlayers; ++i)
				{
					AddPage(m_Pages[i], m_Pages[i]->GetPlayerName());
				}
			}
			else
			{
				// Remove previously added pages
				// we have to manually hide them or they remain visible
				for (size_t i = pageCount - 1; i >= numPlayers; --i)
				{
					m_Pages[i]->Hide();
					RemovePage(i);
				}
			}
		}
	}

protected:
	void OnPageChanged(wxChoicebookEvent& evt)
	{
		if (evt.GetSelection() >= 0 && evt.GetSelection() < (int)GetPageCount())
		{
			static_cast<PlayerNotebookPage*>(GetPage(evt.GetSelection()))->OnDisplay();
		}
		evt.Skip();
	}

private:
	ScenarioEditor& m_ScenarioEditor;
	std::vector<PlayerNotebookPage*> m_Pages;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(PlayerNotebook, wxChoicebook)
	EVT_CHOICEBOOK_PAGE_CHANGED(wxID_ANY, PlayerNotebook::OnPageChanged)
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////////////

class PlayerSettingsControl : public wxPanel
{
public:
	PlayerSettingsControl(wxWindow* parent, ScenarioEditor& scenarioEditor);
	void CreateWidgets();
	void LoadDefaults();
	void ReadFromEngine();
	AtObj UpdateSettingsObject();

private:
	void SendToEngine();

	void OnEdit(wxCommandEvent& WXUNUSED(evt))
	{
		if (!m_InGUIUpdate)
		{
			SendToEngine();
		}
	}

	void OnEditSpin(wxSpinEvent& WXUNUSED(evt))
	{
		if (!m_InGUIUpdate)
		{
			SendToEngine();
		}
	}

	void OnNumPlayersChanged(wxSpinEvent& WXUNUSED(evt))
	{
		if (!m_InGUIUpdate)
		{
			m_Players->ResizePlayers(wxDynamicCast(FindWindow(ID_NumPlayers), wxSpinCtrl)->GetValue());
			SendToEngine();
			
			// Notify observers
			m_MapSettings.NotifyObservers();
		}
	}

	static const size_t MAX_NUM_PLAYERS = 8;

	bool m_InGUIUpdate;
	AtObj m_PlayerDefaults;
	PlayerNotebook* m_Players;
	ScenarioEditor& m_ScenarioEditor;
	std::vector<PlayerPageControls> m_PlayerControls;
	Observable<AtObj>& m_MapSettings;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(PlayerSettingsControl, wxPanel)
	EVT_BUTTON(ID_PlayerColour, PlayerSettingsControl::OnEdit)
	EVT_BUTTON(ID_CameraSet, PlayerSettingsControl::OnEdit)
	EVT_BUTTON(ID_CameraClear, PlayerSettingsControl::OnEdit)
	EVT_CHOICE(wxID_ANY, PlayerSettingsControl::OnEdit)
	EVT_TEXT(wxID_ANY, PlayerSettingsControl::OnEdit)
	EVT_SPINCTRL(ID_NumPlayers, PlayerSettingsControl::OnNumPlayersChanged)
	EVT_SPINCTRL(ID_PlayerFood, PlayerSettingsControl::OnEditSpin)
	EVT_SPINCTRL(ID_PlayerWood, PlayerSettingsControl::OnEditSpin)
	EVT_SPINCTRL(ID_PlayerMetal, PlayerSettingsControl::OnEditSpin)
	EVT_SPINCTRL(ID_PlayerStone, PlayerSettingsControl::OnEditSpin)
	EVT_SPINCTRL(ID_PlayerPop, PlayerSettingsControl::OnEditSpin)
END_EVENT_TABLE();

PlayerSettingsControl::PlayerSettingsControl(wxWindow* parent, ScenarioEditor& scenarioEditor)
	: wxPanel(parent, wxID_ANY), m_ScenarioEditor(scenarioEditor), m_InGUIUpdate(false), m_MapSettings(scenarioEditor.GetMapSettings())
{
	// To prevent recursion, don't handle GUI events right now
	m_InGUIUpdate = true;

	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Player settings"));
	SetSizer(sizer);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizer->Add(new wxStaticText(this, wxID_ANY, _("Num players")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	wxSpinCtrl* numPlayersSpin = new wxSpinCtrl(this, ID_NumPlayers, wxEmptyString, wxDefaultPosition, wxSize(40, -1));
	numPlayersSpin->SetValue(MAX_NUM_PLAYERS);
	numPlayersSpin->SetRange(1, MAX_NUM_PLAYERS);
	boxSizer->Add(numPlayersSpin);
	sizer->Add(boxSizer, wxSizerFlags().Expand().Proportion(0));
	sizer->AddSpacer(5);
	m_Players = new PlayerNotebook(m_ScenarioEditor, this);
	sizer->Add(m_Players, wxSizerFlags().Expand().Proportion(1));

	m_InGUIUpdate = false;
}

void PlayerSettingsControl::CreateWidgets()
{
	// To prevent recursion, don't handle GUI events right now
	m_InGUIUpdate = true;

	// Load default civ and player data
	wxArrayString civNames;
	wxArrayString civCodes;
	AtlasMessage::qGetCivData qryCiv;
	qryCiv.Post();
	std::vector<std::string> civData = *qryCiv.data;
	for (size_t i = 0; i < civData.size(); ++i)
	{
		AtObj civ = AtlasObject::LoadFromJSON(m_ScenarioEditor.GetScriptInterface().GetContext(), civData[i]);
		civNames.Add(wxString(civ["Name"]));
		civCodes.Add(wxString(civ["Code"]));
	}

	// Load AI data
	ArrayOfAIData ais(AIData::CompareAIData);
	AtlasMessage::qGetAIData qryAI;
	qryAI.Post();
	AtObj aiData = AtlasObject::LoadFromJSON(m_ScenarioEditor.GetScriptInterface().GetContext(), *qryAI.data);
	for (AtIter a = aiData["AIData"]["item"]; a.defined(); ++a)
	{
		ais.Add(new AIData(wxString(a["id"]), wxString(a["data"]["name"])));
	}

	// Create player pages
	AtIter player = m_PlayerDefaults["item"];
	++player;	// Skip gaia
	for (size_t i = 0; i < MAX_NUM_PLAYERS; ++i, ++player)
	{
		// Create new player tab and get controls
		PlayerPageControls controls = m_Players->AddPlayer(wxString(player["Name"]), i);
		m_PlayerControls.push_back(controls);

		// Populate civ choice box
		wxChoice* civChoice = controls.civ;
		for (size_t j = 0; j < civNames.Count(); ++j)
			civChoice->Append(civNames[j], new wxStringClientData(civCodes[j]));
		civChoice->SetSelection(0);

		// Populate ai choice box
		wxChoice* aiChoice = controls.ai;
		aiChoice->Append(_("<None>"), new wxStringClientData());
		for (size_t j = 0; j < ais.Count(); ++j)
			aiChoice->Append(ais[j]->GetName(), new wxStringClientData(ais[j]->GetID()));
		aiChoice->SetSelection(0);
	}

	m_InGUIUpdate = false;
}

void PlayerSettingsControl::LoadDefaults()
{
	AtlasMessage::qGetPlayerDefaults qryPlayers;
	qryPlayers.Post();
	AtObj playerData = AtlasObject::LoadFromJSON(m_ScenarioEditor.GetScriptInterface().GetContext(), *qryPlayers.defaults);
	m_PlayerDefaults = *playerData["PlayerData"];
}

void PlayerSettingsControl::ReadFromEngine()
{
	AtlasMessage::qGetMapSettings qry;
	qry.Post();

	if (!(*qry.settings).empty())
	{
		// Prevent error if there's no map settings to parse
		m_MapSettings = AtlasObject::LoadFromJSON(m_ScenarioEditor.GetScriptInterface().GetContext(), *qry.settings);
	}
	else
	{	// Use blank object, it will be created next
		m_MapSettings = AtObj();
	}

	AtIter player = m_MapSettings["PlayerData"]["item"];
	size_t numPlayers = player.count();

	if (!m_MapSettings.defined() || !player.defined() || numPlayers == 0)
	{
		// Player data missing - set number of players manually
		numPlayers = MAX_NUM_PLAYERS;
	}

	// To prevent recursion, don't handle GUI events right now
	m_InGUIUpdate = true;
	
	// Update player controls with player data
	AtIter playerDefs = m_PlayerDefaults["item"];
	++playerDefs;	// skip gaia

	wxDynamicCast(FindWindow(ID_NumPlayers), wxSpinCtrl)->SetValue(numPlayers);

	// Remove / add extra player pages as needed
	m_Players->ResizePlayers(numPlayers);

	for (size_t i = 0; i < numPlayers && i < MAX_NUM_PLAYERS; ++i, ++player, ++playerDefs)
	{
		PlayerPageControls controls = m_PlayerControls[i];

		// name
		wxString name;
		if (player["Name"].defined())
			name = wxString(player["Name"]);
		else
			name = wxString(playerDefs["Name"]);

		controls.name->SetValue(name);

		// civ
		wxChoice* choice = controls.civ;
		wxString civCode(player["Civ"]);
		for (size_t j = 0; j < choice->GetCount(); ++j)
		{
			wxStringClientData* str = dynamic_cast<wxStringClientData*>(choice->GetClientObject(j));
			if (str->GetData() == civCode)
			{
				choice->SetSelection(j);
				break;
			}
		}

		// colour
		wxColour colour;
		AtObj clrObj = *player["Colour"];
		if (clrObj.defined())
		{
			colour = wxColor(wxAtoi(*clrObj["r"]), wxAtoi(*clrObj["g"]), wxAtoi(*clrObj["b"]));
		}
		else
		{
			clrObj = *playerDefs["Colour"];
			colour = wxColor(wxAtoi(*clrObj["r"]), wxAtoi(*clrObj["g"]), wxAtoi(*clrObj["b"]));
		}
		controls.colour->SetBackgroundColour(colour);

		// player type
		wxString aiID;
		if (player["AI"].defined())
			aiID = wxString(player["AI"]);
		else
			aiID = wxString(playerDefs["AI"]);

		choice = controls.ai;
		if (!aiID.empty())
		{	// AI
			for (size_t j = 0; j < choice->GetCount(); ++j)
			{
				wxStringClientData* str = dynamic_cast<wxStringClientData*>(choice->GetClientObject(j));
				if (str->GetData() == aiID)
				{
					choice->SetSelection(j);
					break;
				}
			}
		}
		else
		{	// Human
			choice->SetSelection(0);
		}

		// resources
		AtObj resObj = *player["Resources"];
		if (resObj.defined() && resObj["food"].defined())
			controls.food->SetValue(wxString(resObj["food"]));
		else
			controls.food->SetValue(0);

		if (resObj.defined() && resObj["wood"].defined())
			controls.wood->SetValue(wxString(resObj["wood"]));
		else
			controls.wood->SetValue(0);

		if (resObj.defined() && resObj["metal"].defined())
			controls.metal->SetValue(wxString(resObj["metal"]));
		else
			controls.metal->SetValue(0);

		if (resObj.defined() && resObj["stone"].defined())
			controls.stone->SetValue(wxString(resObj["stone"]));
		else
			controls.stone->SetValue(0);

		// population limit
		if (player["PopulationLimit"].defined())
			controls.pop->SetValue(wxString(player["PopulationLimit"]));
		else
			controls.pop->SetValue(0);

		// team
		if (player["Team"].defined())
			controls.team->SetSelection(wxAtoi(*player["Team"]));
		else
			controls.team->SetSelection(0);

		// camera
		if (player["StartingCamera"].defined())
		{
			sCameraInfo info;
			AtObj camPos = *player["StartingCamera"]["Position"];
			info.pX = wxAtof(*camPos["x"]);
			info.pY = wxAtof(*camPos["y"]);
			info.pZ = wxAtof(*camPos["z"]);
			AtObj camRot = *player["StartingCamera"]["Rotation"];
			info.rX = wxAtof(*camRot["x"]);
			info.rY = wxAtof(*camRot["y"]);
			info.rZ = wxAtof(*camRot["z"]);
			
			controls.page->SetCamera(info, true);
		}
		else
			controls.page->SetCamera(sCameraInfo(), false);

	}

	m_InGUIUpdate = false;
}

AtObj PlayerSettingsControl::UpdateSettingsObject()
{
	// Update player data in the map settings
	AtIter oldPlayer = m_MapSettings["PlayerData"]["item"];
	size_t numPlayers = wxDynamicCast(FindWindow(ID_NumPlayers), wxSpinCtrl)->GetValue();
	AtObj players;
	players.set("@array", L"");

	for (size_t i = 0; i < numPlayers && i < MAX_NUM_PLAYERS; ++i)
	{
		PlayerPageControls controls = m_PlayerControls[i];

		AtObj player = *oldPlayer;

		// name
		wxTextCtrl* text = controls.name;
		if (!text->GetValue().empty())
		{
			player.set("Name", text->GetValue());
		}

		// civ
		wxChoice* choice = controls.civ;
		if (choice->GetSelection() >= 0)
		{
			wxStringClientData* str = dynamic_cast<wxStringClientData*>(choice->GetClientObject(choice->GetSelection()));
			player.set("Civ", str->GetData());
		}

		// colour
		wxColour colour = controls.colour->GetBackgroundColour();
		AtObj clrObj;
		clrObj.setInt("r", (int)colour.Red());
		clrObj.setInt("g", (int)colour.Green());
		clrObj.setInt("b", (int)colour.Blue());
		player.set("Colour", clrObj);

		// player type
		choice = controls.ai;
		if (choice->GetSelection() > 0)
		{	// ai - get id
			wxStringClientData* str = dynamic_cast<wxStringClientData*>(choice->GetClientObject(choice->GetSelection()));
			player.set("AI", str->GetData());
		}
		else
		{	// human
			player.set("AI", _T(""));
		}

		// resources
		AtObj resObj;
		if (controls.food->GetValue() > 0)
			resObj.setInt("food", controls.food->GetValue());
		if (controls.wood->GetValue() > 0)
			resObj.setInt("wood", controls.wood->GetValue());
		if (controls.metal->GetValue() > 0)
			resObj.setInt("metal", controls.metal->GetValue());
		if (controls.stone->GetValue() > 0)
			resObj.setInt("stone", controls.stone->GetValue());
		if (resObj.defined())
			player.set("Resources", resObj);

		// population limit
		if (controls.pop->GetValue() > 0)
			player.setInt("PopulationLimit", controls.pop->GetValue());

		// team
		choice = controls.team;
		if (choice->GetSelection() > 0)
		{
			// valid selection
			player.setInt("Team", choice->GetSelection() - 1);
		}

		// camera
		AtObj camObj;
		if (controls.page->IsCameraDefined())
		{
			sCameraInfo cam = controls.page->GetCamera();
			AtObj camPos;
			camPos.setDouble("x", cam.pX);
			camPos.setDouble("y", cam.pY);
			camPos.setDouble("z", cam.pZ);
			camObj.set("Position", camPos);

			AtObj camRot;
			camRot.setDouble("x", cam.rX);
			camRot.setDouble("y", cam.rY);
			camRot.setDouble("z", cam.rZ);
			camObj.set("Rotation", camRot);
		}
		player.set("StartingCamera", camObj);

		players.add("item", player);
		if (oldPlayer.defined())
			++oldPlayer;
	}
	
	m_MapSettings.set("PlayerData", players);

	return m_MapSettings;
}

void PlayerSettingsControl::SendToEngine()
{
	UpdateSettingsObject();

	std::string json = AtlasObject::SaveToJSON(m_ScenarioEditor.GetScriptInterface().GetContext(), m_MapSettings);

	// TODO: would be nice if we supported undo for settings changes

	POST_COMMAND(SetMapSettings, (json));
}

//////////////////////////////////////////////////////////////////////////

PlayerSidebar::PlayerSidebar(ScenarioEditor& scenarioEditor, wxWindow* sidebarContainer, wxWindow* bottomBarContainer)
	: Sidebar(scenarioEditor, sidebarContainer, bottomBarContainer), m_Loaded(false)
{
	m_PlayerSettingsCtrl = new PlayerSettingsControl(this, m_ScenarioEditor);
	m_MainSizer->Add(m_PlayerSettingsCtrl, wxSizerFlags().Expand());
}

void PlayerSidebar::OnCollapse(wxCollapsiblePaneEvent& WXUNUSED(evt))
{
	Freeze();

	// Toggling the collapsing doesn't seem to update the sidebar layout
	// automatically, so do it explicitly here
	Layout();

	Refresh(); // fixes repaint glitch on Windows

	Thaw();
}

void PlayerSidebar::OnFirstDisplay()
{
	// We do this here becase messages are used which requires simulation to be init'd
	m_PlayerSettingsCtrl->LoadDefaults();
	m_PlayerSettingsCtrl->CreateWidgets();
	m_PlayerSettingsCtrl->ReadFromEngine();

	m_Loaded = true;

	Layout();
}

void PlayerSidebar::OnMapReload()
{
	// Make sure we've loaded the controls
	if (m_Loaded)
	{
		m_PlayerSettingsCtrl->ReadFromEngine();
	}
}

BEGIN_EVENT_TABLE(PlayerSidebar, Sidebar)
	EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY, PlayerSidebar::OnCollapse)
END_EVENT_TABLE();
