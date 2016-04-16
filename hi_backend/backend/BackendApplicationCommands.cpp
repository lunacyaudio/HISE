/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#define toggleVisibility(x) {x->setVisible(!x->isVisible()); owner->setComponentShown(info.commandID, x->isVisible());}

#define SET_COMMAND_TARGET(result, name, active, ticked, shortcut) { result.setInfo(name, name, "Target", 0); \\
															 result.setActive(active); \\
															 result.setTicked(ticked); \\
															 result.addDefaultKeypress(shortcut, ModifierKeys::commandModifier); }


BackendCommandTarget::BackendCommandTarget(BackendProcessor *owner_):
owner(owner_),
currentColumnMode(OneColumn)
{
	

	createMenuBarNames();
}


void BackendCommandTarget::setEditor(BackendProcessorEditor *editor)
{
	bpe = dynamic_cast<BackendProcessorEditor*>(editor);

	mainCommandManager = owner->getCommandManager();
	mainCommandManager->registerAllCommandsForTarget(this);
	mainCommandManager->getKeyMappings()->resetToDefaultMappings();

	bpe->addKeyListener(mainCommandManager->getKeyMappings());
	mainCommandManager->setFirstCommandTarget(this);
	mainCommandManager->commandStatusChanged();
}

void BackendCommandTarget::getAllCommands(Array<CommandID>& commands)
{
	const CommandID id[] = { ModulatorList,
		CustomInterface,
		DebugPanel,
		ViewPanel,
		Mixer,
		Macros,
		Keyboard,
		Settings,
		MenuNewFile,
		MenuOpenFile,
		MenuSaveFile,
		MenuSaveFileAsXmlBackup,
		MenuOpenXmlBackup,
		MenuProjectNew,
		MenuProjectLoad,
		MenuCloseProject,
		MenuProjectShowInFinder,
        MenuFileSaveUserPreset,
		MenuFileSettingsPreset,
		MenuFileSettingsProject,
		MenuFileSettingsUser,
		MenuFileSettingsCompiler,
		MenuFileSettingCheckSanity,
		MenuReplaceWithClipboardContent,
		MenuExportFileAsPlugin,
		MenuFileQuit,
		MenuEditCopy,
		MenuEditPaste,
		MenuEditCreateScriptVariable,
        MenuEditCloseAllChains,
        MenuEditPlotModulator,
		MenuViewShowSelectedProcessorInPopup,
		
		MenuToolsRecompile,
        MenuToolsClearConsole,
		MenuToolsSetCompileTimeOut,
		MenuToolsUseBackgroundThreadForCompile,
		MenuToolsRecompileScriptsOnReload,
		MenuToolsResolveMissingSamples,
		MenuToolsDeleteMissingSamples,
		MenuToolsUseRelativePaths,
		MenuToolsCollectExternalFiles,
        MenuToolsRedirectSampleFolder,
		MenuToolsCreateRSAKeys,
		MenuToolsCreateDummyLicenceFile,
        MenuViewFullscreen,
		MenuViewBack,
		MenuViewForward,
        MenuOneColumn,
		MenuTwoColumns,
		MenuThreeColumns,
		MenuViewShowPluginPopupPreview,
        MenuAddView,
        MenuDeleteView,
        MenuRenameView,
        MenuViewSaveCurrentView,
        MenuToolsCheckDuplicate,
		MenuHelpShowAboutPage,
        MenuHelpCheckVersion
		/*        MenuRevertFile,
		
		
		MenuExportFileAsPlayerLibrary,
		MenuEditOffset,
		MenuViewOffset,
		MenuOneColumn,
		MenuTwoColumns,
		MenuThreeColumns,
		MenuAddView,
		MenuDeleteView,
		MenuRenameView*/
	};

	commands.addArray(id, numElementsInArray(id));
}

void BackendCommandTarget::createMenuBarNames()
{
	menuNames.clear();

	menuNames.add("File");
	menuNames.add("Edit " + (currentCopyPasteTarget.get() == nullptr ? "" : currentCopyPasteTarget->getObjectTypeName()));
	menuNames.add("Tools");
	menuNames.add("View");
	menuNames.add("Help");

	jassert(menuNames.size() == numMenuNames);
}

void BackendCommandTarget::getCommandInfo(CommandID commandID, ApplicationCommandInfo &result)
{
	switch (commandID)
	{

	case CustomInterface: 
		setCommandTarget(result, "Frontend", true, (bpe->interfaceComponent != nullptr && !bpe->interfaceComponent->isVisible()), 'X', false);
		break;
	case Macros:
		setCommandTarget(result, "Show Macro Controls", true, (bpe->macroKnobs != nullptr && !bpe->macroKnobs->isVisible()), 'X', false);
		break;
	case DebugPanel:
		setCommandTarget(result, "Show Debug Panel", true, (bpe->referenceDebugArea != nullptr && !bpe->referenceDebugArea->isVisible()), 'X', false);
		break;
	case ViewPanel:
		setCommandTarget(result, "Show View Panel", true, true, 'X', false);
		break;
	case Mixer:
		setCommandTarget(result, "Show Mixer", false, true, 'X', false);
		break;
	case Keyboard:
		setCommandTarget(result, "Show Keyboard", true, (bpe->keyboard != nullptr && !bpe->keyboard->isVisible()), 'X', false);
		break;
	case Settings:

#if IS_STANDALONE_APP
		setCommandTarget(result, "Show Audio Device Settings", true, bpe->currentDialog == nullptr, 'X', false);
#else
		setCommandTarget(result, "Show Audio Device Settings (disabled for plugins)", false, bpe->currentDialog == nullptr, '8');
#endif
		break;
	case ModulatorList:
		setCommandTarget(result, "Show Processor List", true, (bpe->popupEditor == nullptr), 'x', false);
		break;
	case MenuNewFile:
		setCommandTarget(result, "New File", true, false, 'N');
		break;
	case MenuOpenFile:
		setCommandTarget(result, "Open File", true, false, 'O');
		break;
	case MenuSaveFile:
		setCommandTarget(result, "Save", true, false, 'S');
		break;
	case MenuSaveFileAsXmlBackup:
		setCommandTarget(result, "Save File as XML Backup", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), false, 'X', false);
		break;
	case MenuOpenXmlBackup:
		setCommandTarget(result, "Open XML Backup", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), false, 'X', false);
		break;
	case MenuProjectNew:
		setCommandTarget(result, "Create new Project folder", true, false, 'X', false);
		break;
	case MenuProjectLoad:
		setCommandTarget(result, "Load Project", true, false, 'X', false);
		break;
	case MenuCloseProject:
		setCommandTarget(result, "Close Project", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), false, 'X', false);
		break;
	case MenuProjectShowInFinder:
#if JUCE_WINDOWS
        setCommandTarget(result, "Show Project folder in Explorer",
#else
		setCommandTarget(result, "Show Project folder in Finder",
#endif
        GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), false, 'X', false);
		break;
    case MenuFileSaveUserPreset:
        setCommandTarget(result, "Save current state as new User Preset", true, false, 'X', false);
        break;
    case MenuExportFileAsPlugin:
        setCommandTarget(result, "Export as VST/AU plugin", true, false, 'X', false);
        break;
	case MenuFileSettingsPreset:
		setCommandTarget(result, "Preset Properties", true, false, 'X', false);
		break;
	case MenuFileSettingsProject:
		setCommandTarget(result, "Project Properties", true, false, 'X', false);
		break;
	case MenuFileSettingsUser:
		setCommandTarget(result, "User Settings", true, false, 'X', false);
		break;
	case MenuFileSettingsCompiler:
		setCommandTarget(result, "Compiler Settings", true, false, 'X', false);
		break;
	case MenuFileSettingCheckSanity:
		setCommandTarget(result, "Check for missing properties", true, false, 'X', false);
		break;
	case MenuReplaceWithClipboardContent:
		setCommandTarget(result, "Replace with clipboard content", Actions::hasProcessorInClipboard(), false, 'X', false);
		break;
	case MenuFileQuit:
		setCommandTarget(result, "Quit", true, false, 'X', false); break;
	case MenuEditCopy:
		setCommandTarget(result, "Copy", currentCopyPasteTarget.get() != nullptr, false, 'C');
		break;
	case MenuEditPaste:
		setCommandTarget(result, "Paste", currentCopyPasteTarget.get() != nullptr, false, 'V');
		break;
	case MenuEditCreateScriptVariable:
		setCommandTarget(result, "Create script variable", clipBoardNotEmpty(), false, 'C', true, ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
    case MenuEditPlotModulator:
        {
            BetterProcessorEditor * editor = dynamic_cast<BetterProcessorEditor*>(currentCopyPasteTarget.get());
            
            bool active = false;
            
            bool ticked = false;
            
            if(editor != nullptr)
            {
                Modulator *mod = dynamic_cast<Modulator*>(editor->getProcessor());
                
                if(mod != nullptr)
                {
                    active = true;
                    
                    ticked = mod->isPlotted();
                }
            }
            
            setCommandTarget(result, "Plot Modulator", active, ticked, 'P');
            break;
        }

    case MenuEditCloseAllChains:
        setCommandTarget(result, "Close all chains", clipBoardNotEmpty(), false, 'X', false);
        break;
	case MenuToolsRecompile:
        setCommandTarget(result, "Recompile all scripts", true, false, 'X', false);
        break;
	case MenuToolsSetCompileTimeOut:
		setCommandTarget(result, "Change compile time out duration", true, false, 'X', false);
		break;
	case MenuToolsUseBackgroundThreadForCompile:
		setCommandTarget(result, "Use background thread for script compiling", true, bpe->getBackendProcessor()->isUsingBackgroundThreadForCompiling(), 'X', false);
		break;
	case MenuToolsRecompileScriptsOnReload:
		setCommandTarget(result, "Recompile all scripts on preset load", true, bpe->getBackendProcessor()->isCompilingAllScriptsOnPresetLoad(), 'X', false);
		break;
	case MenuToolsDeleteMissingSamples:
		setCommandTarget(result, "Delete missing samples", true, false, 'X', false);
		break;
	case MenuToolsResolveMissingSamples:
		setCommandTarget(result, "Resolve missing samples", true, false, 'X', false);
		break;
	case MenuToolsUseRelativePaths:
		setCommandTarget(result, "Use relative paths to project folder", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), bpe->getBackendProcessor()->getSampleManager().shouldUseRelativePathToProjectFolder(), 'X', false);
		break;
	case MenuToolsCollectExternalFiles:
		setCommandTarget(result, "Collect external files into Project folder", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(), false, 'X', false);
		break;
    case MenuToolsRedirectSampleFolder:
		setCommandTarget(result, "Redirect sample folder", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive(),
			GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isRedirected(ProjectHandler::SubDirectories::Samples), 'X', false);
        break;
	case MenuToolsCreateRSAKeys:
		setCommandTarget(result, "Create RSA Key pair", true, false, 'X', false);
		break;
	case MenuToolsCreateDummyLicenceFile:
		setCommandTarget(result, "Create Dummy Licence File", true, false, 'X', false);
		break;
    case MenuViewFullscreen:
        setCommandTarget(result, "Toggle Fullscreen", true, bpe->isFullScreenMode(), 'F');
        break;
	case MenuViewBack:
		setCommandTarget(result, "Back: " + bpe->getViewUndoManager()->getUndoDescription(), bpe->getViewUndoManager()->canUndo(), true, (char)KeyPress::backspaceKey, true, ModifierKeys::noModifiers);
		break;
	case MenuViewForward:
		setCommandTarget(result, "Forward: " + bpe->getViewUndoManager()->getRedoDescription(), bpe->getViewUndoManager()->canRedo(), true, (char)KeyPress::backspaceKey, true, ModifierKeys::shiftModifier);
		break;
	case MenuOneColumn:
		setCommandTarget(result, "One Column", true, currentColumnMode == OneColumn, '1', true, ModifierKeys::altModifier);
		break;
	case MenuTwoColumns:
		setCommandTarget(result, "Two Columns", true, currentColumnMode == TwoColumns, '2', true, ModifierKeys::altModifier);
		break;
	case MenuThreeColumns:
		setCommandTarget(result, "Three Columns", true, currentColumnMode == ThreeColumns, '3', true, ModifierKeys::altModifier);
		break;
	case MenuViewShowPluginPopupPreview:
		setCommandTarget(result, "Open Plugin Preview Window", bpe->isPluginPreviewCreatable(), bpe->isPluginPreviewShown(), 'X', false);
		break;
    case MenuAddView:
        setCommandTarget(result, "Add new view", true, false, 'X', false);
        break;
    case MenuDeleteView:
        setCommandTarget(result, "Delete current view", viewActive(), false, 'X', false);
        break;
    case MenuRenameView:
        setCommandTarget(result, "Rename current view", viewActive(), false, 'X', false);
        break;
    case MenuViewSaveCurrentView:
        setCommandTarget(result, "Save current view", viewActive(), false, 'X', false);
        break;
	case MenuViewShowSelectedProcessorInPopup:
		setCommandTarget(result, "Show Processor in full screen", dynamic_cast<BetterProcessorEditor*>(currentCopyPasteTarget.get()) != nullptr, false, 'X', false);
		result.addDefaultKeypress(KeyPress::F11Key, ModifierKeys::noModifiers);
		break;
    case MenuToolsCheckDuplicate:
        setCommandTarget(result, "Check duplicate IDs", true, false, 'X', false);
        break;
    case MenuToolsClearConsole:
        setCommandTarget(result, "Clear Console", true, false, 'X', false);
        break;
	case MenuHelpShowAboutPage:
		setCommandTarget(result, "About HISE", true, false, 'X', false);

		break;
    case MenuHelpCheckVersion:
        setCommandTarget(result, "Check for newer version", true, false, 'X', false);
        break;
            
	default:					jassertfalse; return;
	}
}

bool BackendCommandTarget::perform(const InvocationInfo &info)
{
	switch (info.commandID)
	{
	case CustomInterface:               toggleVisibility(bpe->interfaceComponent);
                                        bpe->viewedComponentChanged();
                                        return true;
	case DebugPanel:                    toggleVisibility(bpe->referenceDebugArea);
                                        bpe->viewedComponentChanged();
                                        return true;
	case Keyboard:                      toggleVisibility(bpe->keyboard);
                                        bpe->viewedComponentChanged();
                                        return true;
	case Macros:                        toggleVisibility(bpe->macroKnobs);
                                        bpe->viewedComponentChanged();
                                        return true;
	case ModulatorList:                 bpe->showModulatorTreePopup(); return true;
	case ViewPanel:                     bpe->showViewPanelPopup(); return true;
	case Settings:                      bpe->showSettingsWindow(); return true;
	case MenuNewFile:                   if (PresetHandler::showYesNoWindow("New File", "Do you want to start a new preset?"))
                                            bpe->clearPreset(); return true;
	case MenuOpenFile:                  Actions::openFile(bpe); return true;
	case MenuSaveFile:                  Actions::saveFile(bpe); return true;
    case MenuSaveFileAsXmlBackup:		Actions::saveFileAsXml(bpe); updateCommands(); return true;
    case MenuOpenXmlBackup:             { FileChooser fc("Select XML file to load",
                                                         GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getSubDirectory(ProjectHandler::SubDirectories::XMLPresetBackups), "*.xml", true);
                                        if (fc.browseForFileToOpen()) Actions::openFileFromXml(bpe, fc.getResult()); return true;}
	case MenuProjectNew:				Actions::createNewProject(bpe); updateCommands();  return true;
	case MenuProjectLoad:				Actions::loadProject(bpe); updateCommands(); return true;
	case MenuCloseProject:				Actions::closeProject(bpe); updateCommands(); return true;
	case MenuProjectShowInFinder:		Actions::showProjectInFinder(bpe); return true;
    case MenuFileSaveUserPreset:        Actions::saveUserPreset(bpe); return true;
	case MenuFileSettingsPreset:		Actions::showFilePresetSettings(bpe); return true;
	case MenuFileSettingsProject:		Actions::showFileProjectSettings(bpe); return true;
	case MenuFileSettingsUser:			Actions::showFileUserSettings(bpe); return true;
	case MenuFileSettingsCompiler:		Actions::showFileCompilerSettings(bpe); return true;
	case MenuFileSettingCheckSanity:	Actions::checkSettingSanity(bpe); return true;
	case MenuReplaceWithClipboardContent: Actions::replaceWithClipboardContent(bpe); return true;
	case MenuFileQuit:                  if (PresetHandler::showYesNoWindow("Quit Application", "Do you want to quit?"))
                                            JUCEApplicationBase::quit(); return true;
	case MenuEditCopy:                  if (currentCopyPasteTarget) currentCopyPasteTarget->copyAction(); return true;
	case MenuEditPaste:                 if (currentCopyPasteTarget) currentCopyPasteTarget->pasteAction(); return true;
	case MenuEditCreateScriptVariable:  Actions::createScriptVariableDeclaration(currentCopyPasteTarget); return true;
    case MenuEditPlotModulator:         Actions::plotModulator(currentCopyPasteTarget.get()); updateCommands(); return true;
    case MenuEditCloseAllChains:        Actions::closeAllChains(bpe); return true;
	case MenuToolsRecompile:            Actions::recompileAllScripts(bpe); return true;
	case MenuToolsSetCompileTimeOut:	Actions::setCompileTimeOut(bpe); return true;
	case MenuToolsUseBackgroundThreadForCompile: Actions::toggleUseBackgroundThreadsForCompiling(bpe); updateCommands(); return true;
	case MenuToolsRecompileScriptsOnReload: Actions::toggleCompileScriptsOnPresetLoad(bpe); updateCommands(); return true;
    case MenuToolsCheckDuplicate:       Actions::checkDuplicateIds(bpe); return true;
	case MenuToolsDeleteMissingSamples: Actions::deleteMissingSamples(bpe); return true;
	case MenuToolsResolveMissingSamples:Actions::resolveMissingSamples(bpe); return true;
	case MenuToolsUseRelativePaths:		Actions::toggleRelativePath(bpe); updateCommands();  return true;
	case MenuToolsCollectExternalFiles:	Actions::collectExternalFiles(bpe); return true;
    case MenuToolsRedirectSampleFolder: Actions::redirectSampleFolder(bpe); updateCommands(); return true;
	case MenuToolsCreateRSAKeys:		Actions::createRSAKeys(bpe); return true;
	case MenuToolsCreateDummyLicenceFile: Actions::createDummyLicenceFile(bpe); return true;
    case MenuViewFullscreen:            Actions::toggleFullscreen(bpe); updateCommands(); return true;
	case MenuViewBack:					bpe->getViewUndoManager()->undo(); updateCommands(); return true;
	case MenuViewForward:				bpe->getViewUndoManager()->redo(); updateCommands(); return true;
	case MenuViewShowPluginPopupPreview: Actions::togglePluginPopupWindow(bpe); updateCommands(); return true;
    case MenuExportFileAsPlugin:        CompileExporter::exportMainSynthChainAsPackage(owner->getMainSynthChain()); return true;
    case MenuAddView:                   Actions::addView(bpe); updateCommands();return true;
    case MenuDeleteView:                Actions::deleteView(bpe); updateCommands();return true;
    case MenuRenameView:                Actions::renameView(bpe); updateCommands();return true;
    case MenuViewSaveCurrentView:       Actions::saveView(bpe); updateCommands(); return true;
    case MenuToolsClearConsole:         owner->clearConsole(); return true;
	case MenuHelpShowAboutPage:			Actions::showAboutPage(bpe); return true;
    case MenuHelpCheckVersion:          Actions::checkVersion(bpe); return true;
	case MenuOneColumn:					Actions::setColumns(bpe, this, OneColumn);  updateCommands(); return true;
	case MenuTwoColumns:				Actions::setColumns(bpe, this, TwoColumns);  updateCommands(); return true;
	case MenuThreeColumns:				Actions::setColumns(bpe, this, ThreeColumns);  updateCommands(); return true;
	case MenuViewShowSelectedProcessorInPopup: Actions::showProcessorInPopup(bpe, dynamic_cast<BetterProcessorEditor*>(currentCopyPasteTarget.get())); return true;
	}

	return false;
}


PopupMenu BackendCommandTarget::getMenuForIndex(int topLevelMenuIndex, const String &/*menuName*/)
{
	MenuNames m = (MenuNames)topLevelMenuIndex;

	PopupMenu p;

	switch (m)
	{
	case BackendCommandTarget::FileMenu: {
		p.addCommandItem(mainCommandManager, MenuNewFile);

		p.addCommandItem(mainCommandManager, MenuOpenFile);
		p.addCommandItem(mainCommandManager, MenuSaveFile);
		p.addCommandItem(mainCommandManager, MenuReplaceWithClipboardContent);

		PopupMenu filesInProject;

		if (GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive())
		{
			GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getFileList(recentFileList, ProjectHandler::SubDirectories::Presets, "*.hip", true);

			for (int i = 0; i < recentFileList.size(); i++)
			{
				filesInProject.addItem(MenuOpenFileFromProjectOffset+i, recentFileList[i].getFileNameWithoutExtension(), true, false);
			}
		}

		p.addSubMenu("Open File from Project", filesInProject, filesInProject.getNumItems() != 0);

		p.addSeparator();

		p.addCommandItem(mainCommandManager, MenuProjectNew);
		p.addCommandItem(mainCommandManager, MenuProjectLoad);
		p.addCommandItem(mainCommandManager, MenuCloseProject);
		p.addCommandItem(mainCommandManager, MenuProjectShowInFinder);

		PopupMenu recentProjects;

		StringArray recentProjectDirectories = GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getRecentWorkDirectories();

		String currentProject = GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getWorkDirectory().getFullPathName();

		for (int i = 0; i < recentProjectDirectories.size(); i++)
		{
			recentProjects.addItem(MenuProjectRecentOffset + i, recentProjectDirectories[i], true, currentProject == recentProjectDirectories[i]);
		}

		p.addSubMenu("Recent projects", recentProjects);

        
        p.addSeparator();
        p.addCommandItem(mainCommandManager, MenuFileSaveUserPreset);
        
        PopupMenu userPresets;
        Array<File> userPresetFiles;
        
		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getFileList(userPresetFiles, ProjectHandler::SubDirectories::UserPresets, "*.preset");
        
        for(int i = 0; i < userPresetFiles.size(); i++)
            userPresets.addItem(i + MenuFileUserPresetMenuOffset, userPresetFiles[i].getFileName());
        
        p.addSubMenu("User Presets", userPresets);
        p.addSeparator();
        
		p.addCommandItem(mainCommandManager, MenuOpenXmlBackup);
		p.addCommandItem(mainCommandManager, MenuSaveFileAsXmlBackup);

        PopupMenu xmlBackups;
        Array<File> xmlBackupFiles;
        
		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getFileList(xmlBackupFiles, ProjectHandler::SubDirectories::XMLPresetBackups, "*.xml");
        
        for(int i = 0; i < xmlBackupFiles.size(); i++)
        {
            xmlBackups.addItem(i + MenuFileXmlBackupMenuOffset, xmlBackupFiles[i].getFileName());
        }
        
        p.addSubMenu("Load XML Backup from Project", xmlBackups);
		p.addSeparator();

		PopupMenu settingsSub;

		settingsSub.addCommandItem(mainCommandManager, MenuFileSettingsPreset);
		settingsSub.addCommandItem(mainCommandManager, MenuFileSettingsProject);
		settingsSub.addCommandItem(mainCommandManager, MenuFileSettingsUser);
		settingsSub.addCommandItem(mainCommandManager, MenuFileSettingsCompiler);
		settingsSub.addSeparator();
		settingsSub.addCommandItem(mainCommandManager, MenuFileSettingCheckSanity);

		p.addSubMenu("Settings", settingsSub);

		PopupMenu exportSub;

        exportSub.addCommandItem(mainCommandManager, MenuExportFileAsPlugin);
		exportSub.addItem(4, "Export as HISE Player library");

		p.addSubMenu("Export", exportSub);
		p.addSeparator();
		p.addCommandItem(mainCommandManager, MenuFileQuit);
		break; }
	case BackendCommandTarget::EditMenu:
        if(dynamic_cast<JavascriptCodeEditor*>(bpe->currentCopyPasteTarget.get()))
        {
            dynamic_cast<JavascriptCodeEditor*>(bpe->currentCopyPasteTarget.get())->addPopupMenuItems(p, nullptr);
            
        }
		else if (dynamic_cast<SampleMapEditor*>(mainCommandManager->getFirstCommandTarget(SampleMapEditor::Undo)))
		{
			dynamic_cast<SampleMapEditor*>(mainCommandManager->getFirstCommandTarget(SampleMapEditor::Undo))->fillPopupMenu(p);
		}
        else
        {
            p.addCommandItem(mainCommandManager, MenuEditCopy);
            p.addCommandItem(mainCommandManager, MenuEditPaste);
            p.addSeparator();
            
            const int chainOffset = 0x6000;
            
            BetterProcessorEditor * editor = dynamic_cast<BetterProcessorEditor*>(bpe->currentCopyPasteTarget.get());
            if(editor != nullptr)
            {
                for(int i = 0; i < editor->getProcessor()->getNumInternalChains(); i++)
                {
                    Processor *child =  editor->getProcessor()->getChildProcessor(i);
                    
                    p.addItem(chainOffset, "Show " + child->getId(), true, child->getEditorState(Processor::EditorState::Visible));
                }
            }
            
            p.addCommandItem(mainCommandManager, MenuEditCreateScriptVariable);
            p.addCommandItem(mainCommandManager, MenuEditCloseAllChains);
            p.addCommandItem(mainCommandManager, MenuEditPlotModulator);
        }
		break;
	case BackendCommandTarget::ToolsMenu:
	{
		p.addSectionHeader("Scripting Tools");
		p.addCommandItem(mainCommandManager, MenuToolsRecompile);
		p.addCommandItem(mainCommandManager, MenuToolsCheckDuplicate);
		p.addCommandItem(mainCommandManager, MenuToolsClearConsole);
		p.addCommandItem(mainCommandManager, MenuToolsRecompileScriptsOnReload);
		p.addCommandItem(mainCommandManager, MenuToolsSetCompileTimeOut);
		p.addCommandItem(mainCommandManager, MenuToolsUseBackgroundThreadForCompile);

		PopupMenu sub;

		Array<File> files;
		StringArray processors;

		bpe->getBackendProcessor()->fillExternalFileList(files, processors);

		for (int i = 0; i < files.size(); i++)
		{
			sub.addItem(MenuToolsExternalScriptFileOffset + i, processors[i] + ": " + files[i].getFileName());
		}

		p.addSubMenu("Edit external script files", sub, files.size() != 0);

		p.addSeparator();
		p.addSectionHeader("Sample Management");
		p.addCommandItem(mainCommandManager, MenuToolsResolveMissingSamples);
		p.addCommandItem(mainCommandManager, MenuToolsDeleteMissingSamples);
		p.addCommandItem(mainCommandManager, MenuToolsUseRelativePaths);
		p.addCommandItem(mainCommandManager, MenuToolsCollectExternalFiles);
		p.addCommandItem(mainCommandManager, MenuToolsRedirectSampleFolder);
		p.addSeparator();
		p.addSectionHeader("Licence Management");
		p.addCommandItem(mainCommandManager, MenuToolsCreateDummyLicenceFile);
		p.addCommandItem(mainCommandManager, MenuToolsCreateRSAKeys);
		break;
	}
	case BackendCommandTarget::ViewMenu: {
		p.addCommandItem(mainCommandManager, MenuViewBack);
		p.addCommandItem(mainCommandManager, MenuViewForward);
		p.addSeparator();
		p.addCommandItem(mainCommandManager, MenuViewFullscreen);
		p.addCommandItem(mainCommandManager, MenuViewShowSelectedProcessorInPopup);
		p.addSeparator();
		p.addCommandItem(mainCommandManager, MenuOneColumn);
		p.addCommandItem(mainCommandManager, MenuTwoColumns);
		p.addCommandItem(mainCommandManager, MenuThreeColumns);
		p.addSeparator();
		p.addCommandItem(mainCommandManager, MenuViewShowPluginPopupPreview);
		p.addCommandItem(mainCommandManager, CustomInterface);
		p.addCommandItem(mainCommandManager, DebugPanel);
		p.addCommandItem(mainCommandManager, Macros);
		p.addCommandItem(mainCommandManager, Keyboard);
		p.addCommandItem(mainCommandManager, Settings);
		p.addSeparator();
		p.addCommandItem(mainCommandManager, MenuAddView);
		p.addCommandItem(mainCommandManager, MenuDeleteView);
		p.addCommandItem(mainCommandManager, MenuRenameView);
		p.addCommandItem(mainCommandManager, MenuViewSaveCurrentView);

		if (viewActive())
		{
			p.addSeparator();
			p.addSectionHeader("Current View: " + owner->synthChain->getCurrentViewInfo()->getViewName());

			for (int i = 0; i < owner->synthChain->getNumViewInfos(); i++)
			{
				ViewInfo *info = owner->synthChain->getViewInfo(i);

				p.addItem(MenuViewOffset + i, info->getViewName(), true, info == owner->synthChain->getCurrentViewInfo());
			}

		}

		PopupMenu processorList;

		Processor::Iterator<Processor> iter(owner->getMainSynthChain());

		int i = 0;

		while (Processor *pl = iter.getNextProcessor())
		{
			if (ProcessorHelpers::is<ModulatorChain>(pl)) continue;
			if (ProcessorHelpers::is<MidiProcessorChain>(pl)) continue;
			if (ProcessorHelpers::is<EffectProcessorChain>(pl)) continue;

			processorList.addItem(MenuViewProcessorListOffset + i, pl->getId());

			i++;
		}

		if (processorList.containsAnyActiveItems())
		{
			p.addSeparator();
			p.addSubMenu("Solo Processor", processorList);
		}

		break;
		}
	case BackendCommandTarget::HelpMenu:
			p.addCommandItem(mainCommandManager, MenuHelpShowAboutPage);
            p.addCommandItem(mainCommandManager, MenuHelpCheckVersion);
		break;
	default:
		break;
	}

	return p;
}

void BackendCommandTarget::menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/)
{
	if (menuItemID >= MenuOpenFileFromProjectOffset && menuItemID < ((int)(MenuOpenFileFromProjectOffset) + 50))
	{
		if (GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive())
		{
			File f = recentFileList[menuItemID - (int)MenuOpenFileFromProjectOffset];

			if (f.existsAsFile())
			{
				bpe->loadNewContainer(f);
			}
		}
	}
	else if (menuItemID >= MenuProjectRecentOffset && menuItemID < (int)MenuProjectRecentOffset + 12)
	{
		const int index = menuItemID - MenuProjectRecentOffset;

		if (PresetHandler::showYesNoWindow("Switch projects?", "Do you want to switch projects? The current preset will be cleared"))
		{
			String file = GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getRecentWorkDirectories()[index];

			bpe->clearPreset();

			GET_PROJECT_HANDLER(bpe->getMainSynthChain()).setWorkingProject(file);
            
            menuItemsChanged();
		}
	}
    else if (menuItemID >= MenuFileUserPresetMenuOffset && menuItemID < ((int)MenuFileUserPresetMenuOffset+50))
    {
        const int index = menuItemID - MenuFileUserPresetMenuOffset;
        
        Array<File> userPresetFiles;
		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getFileList(userPresetFiles, ProjectHandler::SubDirectories::UserPresets, "*.preset");

        File presetToLoad = userPresetFiles[index];
		Actions::loadUserPreset(bpe, presetToLoad);
    }
    else if (menuItemID >= MenuFileXmlBackupMenuOffset && menuItemID < ((int)MenuFileXmlBackupMenuOffset+50))
    {
        const int index = menuItemID - MenuFileXmlBackupMenuOffset;
        
        Array<File> xmlFileList;
        
		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getFileList(xmlFileList, ProjectHandler::SubDirectories::XMLPresetBackups, "*.xml");
        
		File presetToLoad = xmlFileList[index];
            
        Actions::openFileFromXml(bpe, presetToLoad);
    }
    
	else if (menuItemID >= MenuViewOffset && menuItemID < (int)MenuViewOffset + 200)
	{
		ViewInfo *info = owner->synthChain->getViewInfo(menuItemID - MenuViewOffset);

		if (info != nullptr)
		{
			info->restore();

			bpe->setRootProcessor(info->getRootProcessor());

			owner->synthChain->setCurrentViewInfo(menuItemID - MenuViewOffset);
		}
	}

	else if (menuItemID >= MenuViewProcessorListOffset && menuItemID < (int)MenuViewProcessorListOffset + 200)
	{
		Processor::Iterator<Processor> iter(owner->synthChain);

		int i = 0;

		const int indexToLookFor = (menuItemID - (int)MenuViewProcessorListOffset);

		while (Processor *p = iter.getNextProcessor())
		{
			if (ProcessorHelpers::is<ModulatorChain>(p)) continue;
			if (ProcessorHelpers::is<MidiProcessorChain>(p)) continue;
			if (ProcessorHelpers::is<EffectProcessorChain>(p)) continue;

			if (i == indexToLookFor)
			{
				bpe->showProcessorPopup(p, ProcessorHelpers::findParentProcessor(p, false));
			}

			i++;
		}
	}
	else if (menuItemID >= MenuToolsExternalScriptFileOffset && menuItemID < (MenuToolsExternalScriptFileOffset + 50))
	{
		Array<File> files;
		StringArray processors;

		bpe->getBackendProcessor()->fillExternalFileList(files, processors);

		File f = files[menuItemID - MenuToolsExternalScriptFileOffset];

		Processor::Iterator<ScriptProcessor> iter(bpe->getMainSynthChain());

		while (ScriptProcessor *sp = iter.getNextProcessor())
		{
			for (int i = 0; i < sp->getNumWatchedFiles(); i++)
			{
				if (sp->getWatchedFile(i) == f)
				{
					sp->showPopupForFile(i);
					return;
				}
			}
		}

		jassertfalse;
	}
}

#undef toggleVisibility

bool BackendCommandTarget::Actions::hasProcessorInClipboard()
{
	ScopedPointer<XmlElement> xml = XmlDocument::parse(SystemClipboard::getTextFromClipboard());

	if (xml != nullptr)
	{
		ValueTree v = ValueTree::fromXml(*xml);

		if (v.isValid() && v.getProperty("Type") == "SynthChain")
		{
			return true;
			
		}
	}

	return false;
}

void BackendCommandTarget::Actions::openFile(BackendProcessorEditor *bpe)
{
	FileChooser fc("Load Preset File", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getSubDirectory(ProjectHandler::SubDirectories::Presets), "*.hip", true);

	if (fc.browseForFileToOpen()) bpe->loadNewContainer(fc.getResult());
}

void BackendCommandTarget::Actions::saveFile(BackendProcessorEditor *bpe)
{
	if (PresetHandler::showYesNoWindow("Save " + bpe->owner->getMainSynthChain()->getId(), "Do you want to save this preset?"))
	{
		PresetHandler::saveProcessorAsPreset(bpe->owner->getMainSynthChain());
	}
}

void BackendCommandTarget::Actions::replaceWithClipboardContent(BackendProcessorEditor *bpe)
{
	ScopedPointer<XmlElement> xml = XmlDocument::parse(SystemClipboard::getTextFromClipboard());

	if (xml != nullptr)
	{
		ValueTree v = ValueTree::fromXml(*xml);

		if (v.isValid() && v.getProperty("Type") == "SynthChain")
		{
			bpe->loadNewContainer(v);
			return;
		}
	}

	PresetHandler::showMessageWindow("Invalid Preset", "The clipboard does not contain a valid container.");
}

void BackendCommandTarget::Actions::createScriptVariableDeclaration(CopyPasteTarget *currentCopyPasteTarget)
{
	BetterProcessorEditor *editor = dynamic_cast<BetterProcessorEditor*>(currentCopyPasteTarget);

	if (editor != nullptr)
	{
		ProcessorHelpers::getScriptVariableDeclaration(editor->getProcessor());
	}
}

void BackendCommandTarget::Actions::recompileAllScripts(BackendProcessorEditor * bpe)
{
	bpe->owner->compileAllScripts();
}

void BackendCommandTarget::Actions::toggleFullscreen(BackendProcessorEditor * bpe)
{
#if IS_STANDALONE_APP
    
    Component *window = bpe->getParentComponent()->getParentComponent();
    
    if (bpe->isFullScreenMode())
    {
        Desktop::getInstance().setKioskModeComponent(nullptr);
        
        const int height = Desktop::getInstance().getDisplays().getMainDisplay().userArea.getHeight() - 70;
        bpe->setSize(bpe->referenceDebugArea->isVisible() ? 1280 : 900, height);
        bpe->resized();
        
        window->centreWithSize(bpe->getWidth(), bpe->getHeight());
        
        bpe->setAlwaysOnTop(false);
        bpe->borderDragger->setVisible(true);
    }
    else
    {
        Desktop::getInstance().setKioskModeComponent(window);
        
        bpe->borderDragger->setVisible(false);
        bpe->setAlwaysOnTop(true);
        
        bpe->setSize(Desktop::getInstance().getDisplays().getMainDisplay().totalArea.getWidth(),
                Desktop::getInstance().getDisplays().getMainDisplay().totalArea.getHeight());
        
        bpe->resized();
        
    }
#endif
}

void BackendCommandTarget::Actions::addView(BackendProcessorEditor *bpe)
{
    String view = PresetHandler::getCustomName("View");
    
    ViewInfo *info = new ViewInfo(bpe->owner->synthChain, bpe->currentRootProcessor, view);
    
    bpe->owner->synthChain->addViewInfo(info);
}

void BackendCommandTarget::Actions::deleteView(BackendProcessorEditor *bpe)
{
    bpe->owner->synthChain->removeCurrentViewInfo();
}

void BackendCommandTarget::Actions::renameView(BackendProcessorEditor *bpe)
{
    String view = PresetHandler::getCustomName("View");
    
    bpe->owner->synthChain->getCurrentViewInfo()->setViewName(view);
}

void BackendCommandTarget::Actions::saveView(BackendProcessorEditor *bpe)
{
    String view = bpe->owner->synthChain->getCurrentViewInfo()->getViewName();
    
    ViewInfo *info = new ViewInfo(bpe->owner->synthChain, bpe->currentRootProcessor, view);
    
    bpe->owner->synthChain->replaceCurrentViewInfo(info);
}

void BackendCommandTarget::Actions::closeAllChains(BackendProcessorEditor *bpe)
{
    BetterProcessorEditor *editor = dynamic_cast<BetterProcessorEditor*>(bpe->currentCopyPasteTarget.get());
    
    if(editor != nullptr)
    {
        editor->getChainBar()->closeAll();
    }
}

void BackendCommandTarget::Actions::checkDuplicateIds(BackendProcessorEditor *bpe)
{
    PresetHandler::checkProcessorIdsForDuplicates(bpe->owner->synthChain, false);

}

void BackendCommandTarget::Actions::showAboutPage(BackendProcessorEditor * bpe)
{
	bpe->aboutPage->showAboutPage();
}

class UpdateChecker: public ThreadWithAsyncProgressWindow
{
public:

	class ScopedTempFile
	{
	public:

		ScopedTempFile(const File &f_) :
			f(f_)
		{
			f.deleteFile();

			jassert(!f.existsAsFile());

			f.create();
		}

		~ScopedTempFile()
		{
			jassert(f.existsAsFile());

			jassert(f.deleteFile());
		}


		File f;
	};

	UpdateChecker() :
		ThreadWithAsyncProgressWindow("Checking for newer version."),
		updatesAvailable(false),
		lastUpdate(BUILD_SUB_VERSION)
	{
		

		String changelog = getAllChangelogs();

		if (updatesAvailable)
		{
			changelogDisplay = new TextEditor("Changelog");
			changelogDisplay->setSize(500, 300);

			changelogDisplay->setFont(GLOBAL_MONOSPACE_FONT());
			changelogDisplay->setMultiLine(true);
			changelogDisplay->setReadOnly(true);
			changelogDisplay->setText(changelog);
			addCustomComponent(changelogDisplay);

			filePicker = new FilenameComponent("Download Location", File::getSpecialLocation(File::SpecialLocationType::userDesktopDirectory), false, true, true, "", "", "Choose Download Location");
			filePicker->setSize(500, 24);

			addCustomComponent(filePicker);

			addBasicComponents();

			showStatusMessage("New build available: " + String(lastUpdate) + ". Press OK to download file to the selected location");
		}
		else
		{
			addBasicComponents(false);

			showStatusMessage("Your HISE build is up to date.");
		}
	}

	static bool downloadProgress(void* context, int bytesSent, int totalBytes)
	{
		const double downloadedMB = (double)bytesSent / 1024.0 / 1024.0;
		const double totalMB = (double)totalBytes / 1024.0 / 1024.0;
		const double percent = downloadedMB / totalMB;

		static_cast<UpdateChecker*>(context)->showStatusMessage("Downloaded: " + String(downloadedMB, 2) + " MB / " + String(totalMB, 2) + " MB");

		static_cast<UpdateChecker*>(context)->setProgress(percent);

		return !static_cast<UpdateChecker*>(context)->threadShouldExit();
	}

	void run()
	{
		static String webFolder("http://hartinstruments.net/hise/download/nightly_builds/");

		static String version = "099";

#if JUCE_WINDOWS
		String downloadFileName = "HISE_" + version + "_build" + String(lastUpdate) + ".exe";
#else
		String downloadFileName = "HISE_" + version + "_build" + String(lastUpdate) + "_OSX.dmg";
#endif

		URL url(webFolder + downloadFileName);

		ScopedPointer<InputStream> stream = url.createInputStream(false, &downloadProgress, this);

		target = File(filePicker->getCurrentFile().getChildFile(downloadFileName));

		if (!target.existsAsFile())
		{
			MemoryBlock mb;

			mb.setSize(8192);

			ScopedPointer<ScopedTempFile> tempFile = new ScopedTempFile(File(target.getFullPathName()+"temp"));

			ScopedPointer<FileOutputStream> fos = new FileOutputStream(tempFile->f);

			const int64 numBytesTotal = stream->getNumBytesRemaining();

			int64 numBytesRead = 0;

			downloadOK = false;

			while (stream->getNumBytesRemaining() > 0)
			{
				const int64 chunkSize = jmin<int64>(stream->getNumBytesRemaining(), 8192);

				downloadProgress(this, (int)numBytesRead, (int)numBytesTotal);

				if (threadShouldExit())
				{
					fos->flush();
					fos = nullptr;

					tempFile = nullptr;
					return;
				}

				stream->read(mb.getData(), (int)chunkSize);

				numBytesRead += chunkSize;

				fos->write(mb.getData(), (size_t)chunkSize);
			}

			downloadOK = true;
			fos->flush();
			
			tempFile->f.copyFileTo(target);
		}
	};

	void threadFinished()
	{
		if (downloadOK)
		{
			PresetHandler::showMessageWindow("Download finished", "Quit the app and run the installer to update to the latest version");

			target.revealToUser();
		}
	}

private:

	String getAllChangelogs()
	{
		int latestVersion = BUILD_SUB_VERSION + 1;

		String completeLog;

		bool lookFurther = true;

		while (lookFurther)
		{
			String log = getChangelog(latestVersion);

			if (log == "404")
			{
				lookFurther = false;
				continue;
			}

			updatesAvailable = true;

			lastUpdate = latestVersion;

			completeLog << log;
			latestVersion++;
		}

		return completeLog;
	}

	String getChangelog(int i) const
	{
		static String webFolder("http://www.hartinstruments.net/hise/download/nightly_builds/");

		String changelogFile("changelog_" + String(i) + ".txt");

		URL url(webFolder + changelogFile);

		String content;

		ScopedPointer<InputStream> stream = url.createInputStream(false);

        if(stream != nullptr)
        {
            String changes = stream->readEntireStreamAsString();
            
            if (changes.contains("404"))
            {
                return "404";
            }
            else if (changes.containsNonWhitespaceChars())
            {
                content = "Build " + String(i) + ":\n\n" + changes;
                return content;
            }
            
            return String::empty;
        }
        else
        {
            return "404";
        }
	}

	bool updatesAvailable;

	int lastUpdate;

	File target;
	ScopedPointer<ScopedTempFile> tempFile;

	bool downloadOK;

	ScopedPointer<FilenameComponent> filePicker;
	ScopedPointer<TextEditor> changelogDisplay;
};


static bool canConnectToWebsite (const URL& url)
{
    ScopedPointer<InputStream> in (url.createInputStream (false, nullptr, nullptr, String(), 2000, nullptr));
    return in != nullptr;
}

static bool areMajorWebsitesAvailable()
{
    if (canConnectToWebsite (URL ("http://hartinstruments.net")))
        return true;
    
    return false;
}

void BackendCommandTarget::Actions::checkVersion(BackendProcessorEditor *bpe)
{
    if (areMajorWebsitesAvailable())
    {
        UpdateChecker * checker = new UpdateChecker();
        
        checker->setModalComponentOfMainEditor(bpe);
    }
    else
    {
        PresetHandler::showMessageWindow("Offline", "Could not connect to the server");
    }
}

void BackendCommandTarget::Actions::setColumns(BackendProcessorEditor * bpe, BackendCommandTarget* target, ColumnMode columns)
{
	target->currentColumnMode = columns;

	switch (columns)
	{
	case BackendCommandTarget::OneColumn:
		bpe->setSize(900, bpe->getHeight());
		break;
	case BackendCommandTarget::TwoColumns:
		bpe->setSize(jmin<int>(Desktop::getInstance().getDisplays().getMainDisplay().totalArea.getWidth(), 1280), bpe->getHeight());
		break;
	case BackendCommandTarget::ThreeColumns:
		bpe->setSize(jmin<int>(Desktop::getInstance().getDisplays().getMainDisplay().totalArea.getWidth(), 1650), bpe->getHeight());
		break;
	default:
		break;
	}

	bpe->resized();
	
}

void BackendCommandTarget::Actions::showProcessorInPopup(BackendProcessorEditor * bpe, BetterProcessorEditor* editor)
{
	bpe->showProcessorPopup(editor->getProcessor(), editor->getParentEditor() != nullptr ? editor->getParentEditor()->getProcessor() : nullptr);
}

void BackendCommandTarget::Actions::plotModulator(CopyPasteTarget *currentCopyPasteTarget)
{
    BetterProcessorEditor *editor = dynamic_cast<BetterProcessorEditor*>(currentCopyPasteTarget);
                                                                         
    if(editor != nullptr)
    {
        Modulator *mod = dynamic_cast<Modulator*>(editor->getProcessor());
        if(mod != nullptr)
        {
            if(mod->isPlotted())
            {
                mod->getMainController()->removePlottedModulator(mod);
            }
            else
            {
                mod->getMainController()->addPlottedModulator(mod);
            }
        }
    }
}

void BackendCommandTarget::Actions::resolveMissingSamples(BackendProcessorEditor *bpe)
{
	bpe->getBackendProcessor()->getSampleManager().getModulatorSamplerSoundPool()->resolveMissingSamples(bpe);
}

void BackendCommandTarget::Actions::deleteMissingSamples(BackendProcessorEditor *bpe)
{
	bpe->getBackendProcessor()->getSampleManager().getModulatorSamplerSoundPool()->deleteMissingSamples();
}

void BackendCommandTarget::Actions::setCompileTimeOut(BackendProcessorEditor * bpe)
{
	AlertWindowLookAndFeel alaf;

	AlertWindow newTime("Enter new compile time out duration", "Current time out: " + String(bpe->getBackendProcessor()->getCompileTimeOut(),1) + " seconds.", AlertWindow::QuestionIcon, bpe);

	newTime.setLookAndFeel(&alaf);

	newTime.addTextEditor("time", String(bpe->getBackendProcessor()->getCompileTimeOut(), 1), "", false);
	
	newTime.addButton("OK", 1, KeyPress(KeyPress::returnKey));
	newTime.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	if (newTime.runModalLoop())
	{
		double value = newTime.getTextEditor("time")->getText().getDoubleValue();

		if (value != 0.0)
		{
			bpe->getBackendProcessor()->setCompileTimeOut(value);
		}
	}
}

void BackendCommandTarget::Actions::toggleUseBackgroundThreadsForCompiling(BackendProcessorEditor * bpe)
{
	const bool lastState = bpe->getBackendProcessor()->isUsingBackgroundThreadForCompiling();

	bpe->getBackendProcessor()->setShouldUseBackgroundThreadForCompiling(!lastState);
}

void BackendCommandTarget::Actions::toggleCompileScriptsOnPresetLoad(BackendProcessorEditor * bpe)
{
	const bool lastState = bpe->getBackendProcessor()->isCompilingAllScriptsOnPresetLoad();

	bpe->getBackendProcessor()->setEnableCompileAllScriptsOnPresetLoad(!lastState);
}


void BackendCommandTarget::Actions::toggleRelativePath(BackendProcessorEditor * bpe)
{
	const bool state = bpe->getBackendProcessor()->getSampleManager().shouldUseRelativePathToProjectFolder();

	bpe->getBackendProcessor()->getSampleManager().setShouldUseRelativePathToProjectFolder(!state);
}


void BackendCommandTarget::Actions::collectExternalFiles(BackendProcessorEditor * bpe)
{
	ExternalResourceCollector *resource = new ExternalResourceCollector(bpe->getBackendProcessor());

	resource->setModalComponentOfMainEditor(bpe);
}

struct XmlBackupFunctions
{
	static void removeEditorStatesFromXml(XmlElement &xml)
	{
		xml.deleteAllChildElementsWithTagName("EditorStates");

		for (int i = 0; i < xml.getNumChildElements(); i++)
		{
			removeEditorStatesFromXml(*xml.getChildElement(i));
		}
	}

	static void removeAllScripts(XmlElement &xml)
	{
		xml.removeAttribute("Script");

		for (int i = 0; i < xml.getNumChildElements(); i++)
		{
			removeAllScripts(*xml.getChildElement(i));
		}
	}

	static void restoreAllScripts(XmlElement &xml, ModulatorSynthChain *masterChain, const String &newId)
	{
		if (xml.hasTagName("Processor") && xml.getStringAttribute("Type") == "ScriptProcessor")
		{
			File scriptDirectory = getScriptDirectoryFor(masterChain, newId);

			DirectoryIterator iter(scriptDirectory, false, "*.js", File::findFiles);

			while (iter.next())
			{
				File script = iter.getFile();

				if (script.getFileNameWithoutExtension() == getSanitiziedName(xml.getStringAttribute("ID")))
				{
					xml.setAttribute("Script", script.loadFileAsString());
					break;
				}
			}
		}

		for (int i = 0; i < xml.getNumChildElements(); i++)
		{
			restoreAllScripts(*xml.getChildElement(i), masterChain, newId);
		}
	}

	static File getScriptDirectoryFor(ModulatorSynthChain *masterChain, const String &chainId=String::empty)
	{
		if (chainId.isEmpty())
		{
			return GET_PROJECT_HANDLER(masterChain).getSubDirectory(ProjectHandler::SubDirectories::Scripts).getChildFile("ScriptProcessors/" + getSanitiziedName(masterChain->getId()));
		}
		else
		{
			return GET_PROJECT_HANDLER(masterChain).getSubDirectory(ProjectHandler::SubDirectories::Scripts).getChildFile("ScriptProcessors/" + getSanitiziedName(chainId));
		}
	}

	static File getScriptFileFor(ModulatorSynthChain *masterChain, const String &id)
	{
		return getScriptDirectoryFor(masterChain).getChildFile(getSanitiziedName(id) + ".js");
	}

private:

	static String getSanitiziedName(const String &id)
	{
		return id.removeCharacters(" .()");
	}
};


void BackendCommandTarget::Actions::saveFileAsXml(BackendProcessorEditor * bpe)
{
	if (GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive())
	{
		FileChooser fc("Select XML file to save", GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getSubDirectory(ProjectHandler::SubDirectories::XMLPresetBackups), "*.xml", true);

		if (fc.browseForFileToSave(true))
		{
			ValueTree v = bpe->owner->getMainSynthChain()->exportAsValueTree();

            v.setProperty("BuildVersion", BUILD_SUB_VERSION, nullptr);
            
			ScopedPointer<XmlElement> xml = v.createXml();

			if(PresetHandler::showYesNoWindow("Strip Editor View information", "Do you want to strip the editor view information? This reduces the noise when using version controls systems."))
			{
				XmlBackupFunctions::removeEditorStatesFromXml(*xml);
			}

			if (PresetHandler::showYesNoWindow("Write Scripts to external file?", "Do you want to write the scripts into an external file (They will be imported when loading this backup"))
			{
				File scriptDirectory = XmlBackupFunctions::getScriptDirectoryFor(bpe->getMainSynthChain());

				Processor::Iterator<ScriptProcessor> iter(bpe->getMainSynthChain());

				scriptDirectory.deleteRecursively();

				scriptDirectory.createDirectory();

				while (ScriptProcessor *sp = iter.getNextProcessor())
				{
					String content;

					sp->mergeCallbacksToScript(content);

					File scriptFile = XmlBackupFunctions::getScriptFileFor(bpe->getMainSynthChain(), sp->getId());

					scriptFile.replaceWithText(content);
				}

				XmlBackupFunctions::removeAllScripts(*xml);
			}

			fc.getResult().replaceWithText(xml->createDocument(""));

			debugToConsole(bpe->owner->getMainSynthChain(), "Exported as XML");
            
            
		}
	}
}

void BackendCommandTarget::Actions::openFileFromXml(BackendProcessorEditor * bpe, const File &fileToLoad)
{
	if (GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive())
	{
		ScopedPointer<XmlElement> xml = XmlDocument::parse(fileToLoad);

		String newId = xml->getStringAttribute("ID");

		XmlBackupFunctions::restoreAllScripts(*xml, bpe->getMainSynthChain(), newId);

		ValueTree v = ValueTree::fromXml(*xml);

		bpe->loadNewContainer(v);
	}
}

void BackendCommandTarget::Actions::createNewProject(BackendProcessorEditor *bpe)
{
	FileChooser fc("Create new project directory");

	if (fc.browseForDirectory())
	{
		File f = fc.getResult();

		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).createNewProject(f);
	}
}

void BackendCommandTarget::Actions::loadProject(BackendProcessorEditor *bpe)
{
	FileChooser fc("Load project (set as working directory)");

	if (fc.browseForDirectory())
	{
		File f = fc.getResult();

		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).setWorkingProject(f);
	}
}

void BackendCommandTarget::Actions::closeProject(BackendProcessorEditor *bpe)
{
	GET_PROJECT_HANDLER(bpe->getMainSynthChain()).setWorkingProject(File::nonexistent);
}

void BackendCommandTarget::Actions::showProjectInFinder(BackendProcessorEditor *bpe)
{
	if (GET_PROJECT_HANDLER(bpe->getMainSynthChain()).isActive())
	{
		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).getWorkDirectory().revealToUser();
	}
}

void BackendCommandTarget::Actions::saveUserPreset(BackendProcessorEditor *bpe)
{
    UserPresetHandler::saveUserPreset(bpe->getMainSynthChain());
}

void BackendCommandTarget::Actions::loadUserPreset(BackendProcessorEditor *bpe, const File &fileToLoad)
{
    UserPresetHandler::loadUserPreset(bpe->getMainSynthChain(), fileToLoad);
}

void BackendCommandTarget::Actions::redirectSampleFolder(BackendProcessorEditor *bpe)
{
    FileChooser fc("Redirect sample folder to the following location");
    
    if (fc.browseForDirectory())
    {
        File f = fc.getResult();
        
		GET_PROJECT_HANDLER(bpe->getMainSynthChain()).createLinkFile(ProjectHandler::SubDirectories::Samples, f);
    }
}

void BackendCommandTarget::Actions::showFilePresetSettings(BackendProcessorEditor * /*bpe*/)
{
	
}

void BackendCommandTarget::Actions::showFileProjectSettings(BackendProcessorEditor * bpe)
{
	SettingWindows::ProjectSettingWindow *window = new SettingWindows::ProjectSettingWindow(&GET_PROJECT_HANDLER(bpe->getMainSynthChain()));

	window->setModalComponentOfMainEditor(bpe);
}

void BackendCommandTarget::Actions::showFileUserSettings(BackendProcessorEditor * bpe)
{
	SettingWindows::UserSettingWindow *window = new SettingWindows::UserSettingWindow();

	window->setModalComponentOfMainEditor(bpe);
}

void BackendCommandTarget::Actions::showFileCompilerSettings(BackendProcessorEditor * bpe)
{
	SettingWindows::CompilerSettingWindow *window = new SettingWindows::CompilerSettingWindow();

	window->setModalComponentOfMainEditor(bpe);
}

void BackendCommandTarget::Actions::checkSettingSanity(BackendProcessorEditor * bpe)
{
	SettingWindows::checkAllSettings(&GET_PROJECT_HANDLER(bpe->getMainSynthChain()));
}

void BackendCommandTarget::Actions::togglePluginPopupWindow(BackendProcessorEditor * bpe)
{
	if (bpe->isPluginPreviewShown())
	{
		bpe->setPluginPreviewWindow(nullptr);
	}
	else
	{
		bpe->setPluginPreviewWindow(new PluginPreviewWindow(bpe));
	}
}

void BackendCommandTarget::Actions::createRSAKeys(BackendProcessorEditor * bpe)
{
	GET_PROJECT_HANDLER(bpe->getMainSynthChain()).createRSAKey();
}

class DummyUnlocker : public OnlineUnlockStatus
{
public:

	DummyUnlocker(ProjectHandler *handler_):
		handler(handler_)
	{

	}
	
	String getProductID() override
	{
		return SettingWindows::getSettingValue((int)SettingWindows::ProjectSettingWindow::Attributes::Name, handler);
	}

	bool doesProductIDMatch(const String & 	returnedIDFromServer)
	{
		return returnedIDFromServer == getProductID();
	}

	


private:

	ProjectHandler *handler;


};

void BackendCommandTarget::Actions::createDummyLicenceFile(BackendProcessorEditor * bpe)
{
	ProjectHandler *handler = &GET_PROJECT_HANDLER(bpe->getMainSynthChain());

	if (!handler->isActive())
	{
		PresetHandler::showMessageWindow("No Project active", "You need an active project to create a key file.");
		return;
	}

	const String appName = SettingWindows::getSettingValue((int)SettingWindows::ProjectSettingWindow::Attributes::Name, handler);

	if (appName.isEmpty())
	{
		PresetHandler::showMessageWindow("No Product name", "You need a product name for a licence file.");
		return;
	}

	const String dummyEmail = "dummy@email.com";
	const String userName = "Dummy McLovin";

	StringArray ids;

#if JUCE_WINDOWS
	OnlineUnlockStatus::MachineIDUtilities::addFileIDToList(ids, File::getSpecialLocation(File::windowsSystemDirectory));
#else
	OnlineUnlockStatus::MachineIDUtilities::addFileIDToList(ids, File("~"));
#endif

	// ..if that fails, use the MAC addresses..
	if (ids.size() == 0)
		OnlineUnlockStatus::MachineIDUtilities::addMACAddressesToList(ids);

	RSAKey privateKey = RSAKey(handler->getPrivateKey());

	if (!privateKey.isValid())
	{
		PresetHandler::showMessageWindow("No RSA key", "You have to create a RSA Key pair first.");
		return;
	}

	String keyContent = KeyGeneration::generateKeyFile(appName, dummyEmail, userName, ids.joinIntoString("\n"), privateKey);

	File key = handler->getWorkDirectory().getChildFile(appName + ".licence");

	key.replaceWithText(keyContent);
}
