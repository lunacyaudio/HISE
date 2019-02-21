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
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hise.audio/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for closed source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

namespace hise { using namespace juce;

const FloatingTile* FloatingTileContainer::getComponent(int index) const
{
	return components[index];
}

FloatingTile* FloatingTileContainer::getComponent(int index)
{
	return components[index];
}

int FloatingTileContainer::getNumComponents() const
{
	return components.size();
}

void FloatingTileContainer::clear()
{
	// if this doesn't work, you're in trouble soon...
	jassert(dynamic_cast<Component*>(this) != nullptr);

	const int numToClear = components.size();

	for (int i = 0; i < numToClear; i++)
	{
		removeFloatingTile(components.getLast());
	}
}

int FloatingTileContainer::getIndexOfComponent(const FloatingTile* componentToLookFor) const
{
	return components.indexOf(componentToLookFor);
}



void FloatingTileContainer::addFloatingTile(FloatingTile* newComponent)
{
	components.add(newComponent);

	componentAdded(newComponent);

	newComponent->refreshRootLayout();
}

void FloatingTileContainer::removeFloatingTile(FloatingTile* componentToRemove)
{
	ScopedPointer<FloatingTile> temp = components.removeAndReturn(components.indexOf(componentToRemove));

	componentRemoved(temp.get());

	temp.get()->refreshRootLayout();

	temp = nullptr;
}

bool FloatingTileContainer::shouldIntendAddButton() const
{
	return FloatingTile::LayoutHelpers::showFoldButton(getParentShell());
}

var FloatingTileContainer::toDynamicObject() const
{
	var obj = FloatingTileContent::toDynamicObject();

	storePropertyInObject(obj, ContainerPropertyIds::Dynamic, dynamic, true);

	Array<var> children;
	children.ensureStorageAllocated(getNumComponents());

	for (int i = 0; i < getNumComponents(); i++)
	{
		var child = getComponent(i)->getCurrentFloatingPanel()->toDynamicObject();
		children.add(child);
	}

	storePropertyInObject(obj, ContainerPropertyIds::Content, children);

	return obj;
}

void FloatingTileContainer::fromDynamicObject(const var& objectData)
{
	FloatingTileContent::fromDynamicObject(objectData);

	setIsDynamic(getPropertyWithDefault(objectData, ContainerPropertyIds::Dynamic));

	clear();

	var children = getPropertyWithDefault(objectData, ContainerPropertyIds::Content);

	if (auto childList = children.getArray())
	{
		for (int i = 0; i < childList->size(); i++)
		{
			auto newShell = new FloatingTile(getParentShell()->getMainController(), this, childList->getUnchecked(i));
			addFloatingTile(newShell);
		}
	}
}

int FloatingTileContainer::getNumDefaultableProperties() const
{
	return ContainerPropertyIds::numContainerPropertyIds;
}

Identifier FloatingTileContainer::getDefaultablePropertyId(int index) const
{
	if (index < (int)PanelPropertyId::numPropertyIds)
		return FloatingTileContent::getDefaultablePropertyId(index);

	RETURN_DEFAULT_PROPERTY_ID(index, ContainerPropertyIds::Content, "Content");
	RETURN_DEFAULT_PROPERTY_ID(index, ContainerPropertyIds::Dynamic, "Dynamic");

	jassertfalse;
	return Identifier();
}

var FloatingTileContainer::getDefaultProperty(int id) const
{
	if (id < (int)PanelPropertyId::numPropertyIds)
		return FloatingTileContent::getDefaultProperty(id);

	auto prop = (ContainerPropertyIds)id;

	switch (prop)
	{
	case FloatingTileContainer::Dynamic: return true;
	case FloatingTileContainer::Content: return Array<var>();
	default: break;
	}

	jassertfalse;
	return var();
}

void FloatingTileContainer::enableSwapMode(bool shouldBeSwappable, FloatingTile* source)
{
	resizeSource = shouldBeSwappable ? source : nullptr;

	for (int i = 0; i < components.size(); i++)
	{
		components[i]->enableSwapMode(shouldBeSwappable, resizeSource.getComponent());
	}
}

void FloatingTileContainer::refreshLayout()
{
	FloatingTile::Iterator<FloatingTileContainer> iter(getParentShell());

	// skip yourself...
	iter.getNextPanel();

	while (auto c = iter.getNextPanel())
		c->refreshLayout();

}

void FloatingTileContainer::notifySiblingChange()
{
	for (int i = 0; i < getNumComponents(); i++)
	{
		getComponent(i)->getCurrentFloatingPanel()->siblingAmountChanged();
	}
}

FloatingTabComponent::CloseButton::CloseButton() :
	ShapeButton("Close", Colours::white.withAlpha(0.2f), Colours::white.withAlpha(0.8f), Colours::white)
{
	Path p;
	p.loadPathFromData(HiBinaryData::ProcessorEditorHeaderIcons::closeIcon, sizeof(HiBinaryData::ProcessorEditorHeaderIcons::closeIcon));
	setShape(p, false, true, true);

	addListener(this);
}

void FloatingTabComponent::CloseButton::buttonClicked(Button* )
{
	TabBarButton* button = findParentComponentOfClass<TabBarButton>();
	auto* tab = findParentComponentOfClass<FloatingTabComponent>();

	auto cToRemove = tab->getComponent(button->getIndex());

	if (cToRemove->isEmpty() || PresetHandler::showYesNoWindow("Delete tab", "Do you want to delete the tab " + cToRemove->getCurrentFloatingPanel()->getTitle()))
		tab->removeFloatingTile(cToRemove);
}

int FloatingTabComponent::LookAndFeel::getTabButtonBestWidth(TabBarButton &b, int )
{
	auto w = GLOBAL_BOLD_FONT().getStringWidthFloat(b.getButtonText());

	return (int)(w + 48);
}


Path createTabBackgroundPath(Rectangle<float> bounds)
{

	Path p;

	p.startNewSubPath(bounds.getX(), bounds.getY() + 4);
	p.lineTo(bounds.getX() + 3, bounds.getY()+1);
	p.lineTo(bounds.getRight() - 3, bounds.getY()+1);
	p.lineTo(bounds.getRight(), bounds.getY() + 4);
	p.lineTo(bounds.getRight(), bounds.getBottom());
	p.lineTo(bounds.getX(), bounds.getBottom());
	p.closeSubPath();

	return p;
}

void FloatingTabComponent::LookAndFeel::drawTabButton(TabBarButton &b, Graphics &g, bool isMouseOver, bool /*isMouseDown*/)
{
	if (isMouseOver)
	{
		g.setColour(JUCE_LIVE_CONSTANT_OFF(Colour(0x13ffffff)));
		g.fillPath(createTabBackgroundPath(Rectangle<float>(0.0f, 0.0f, (float)b.getWidth(), 20.0f)));
	}

	g.setColour(Colours::black.withAlpha(0.1f));

	auto c = b.findParentComponentOfClass<FloatingTileContent>()->findPanelColour(FloatingTileContent::PanelColourId::textColour);

	g.setColour(c);
	g.setFont(GLOBAL_BOLD_FONT());

	g.drawText(b.getButtonText(), 5, 2, b.getWidth() - 10, b.getHeight() - 4, Justification::centredLeft);
}


Rectangle< int > FloatingTabComponent::LookAndFeel::getTabButtonExtraComponentBounds(const TabBarButton &b, Rectangle< int > &/*textArea*/, Component &/*extraComp*/)
{
	return Rectangle<int>(b.getWidth() - 18, 2, 16, 16);
}


void FloatingTabComponent::LookAndFeel::drawTabAreaBehindFrontButton(TabbedButtonBar &b, Graphics &g, int , int )
{
	

	if (b.getCurrentTabIndex() != -1)
	{

		auto c = b.findParentComponentOfClass<FloatingTileContent>()->findPanelColour(FloatingTileContent::PanelColourId::itemColour1);

		g.setColour(c);

		auto bounds = FLOAT_RECTANGLE(b.getTabButton(b.getCurrentTabIndex())->getBoundsInParent());

		g.fillPath(createTabBackgroundPath(bounds));
	}
		
}

FloatingTabComponent::FloatingTabComponent(FloatingTile* parent) :
	FloatingTileContainer(parent),
	TabbedComponent(TabbedButtonBar::TabsAtTop)
{
	setDefaultPanelColour(PanelColourId::bgColour, HiseColourScheme::getColour(HiseColourScheme::EditorBackgroundColourId));
	setDefaultPanelColour(PanelColourId::itemColour1, HiseColourScheme::getColour(HiseColourScheme::EditorBackgroundColourIdBright));
	setDefaultPanelColour(PanelColourId::textColour, Colours::white);


	addAndMakeVisible(addButton = new ShapeButton("Add Column", Colours::white.withAlpha(0.7f), Colours::white, Colours::white));

	Path p;
	p.loadPathFromData(HiBinaryData::ProcessorEditorHeaderIcons::addIcon, sizeof(HiBinaryData::ProcessorEditorHeaderIcons::addIcon));

	addButton->setShape(p, false, false, true);

	addButton->addListener(this);

	setOutline(0);

	setTabBarDepth(24);

	getTabbedButtonBar().setLookAndFeel(&laf);

	setColour(TabbedComponent::ColourIds::outlineColourId, Colours::transparentBlack);

	addFloatingTile(new FloatingTile(parent->getMainController(), this));
}

FloatingTabComponent::~FloatingTabComponent()
{
	clear();
}

void FloatingTabComponent::popupMenuClickOnTab(int tabIndex, const String& /*tabName*/)
{
	PopupMenu m;

	m.setLookAndFeel(&plaf);

	m.addItem(1, "Rename Tab", !getComponent(tabIndex)->isVital());
	m.addSeparator();
	m.addItem(2, "Export Tab as JSON", !getComponent(tabIndex)->isVital());
	m.addItem(3, "Replace Tab with JSON in clipboard", !getComponent(tabIndex)->isVital());

	const int result = m.show();

	if (result == 1)
	{
		auto newName = PresetHandler::getCustomName("Tab", "Enter the tab name");
		getComponent(tabIndex)->getCurrentFloatingPanel()->setCustomTitle(newName);
		
		getTabbedButtonBar().repaint();
	}
	else if (result == 2)
	{
		SystemClipboard::copyTextToClipboard(getComponent(tabIndex)->exportAsJSON());
	}
	else if (result == 3)
	{
		getComponent(tabIndex)->loadFromJSON(SystemClipboard::getTextFromClipboard());
	}
}

void FloatingTabComponent::refreshLayout()
{
	FloatingTileContainer::refreshLayout();

	if (getCurrentTabIndex() != -1)
	{
		getComponent(getCurrentTabIndex())->resized();
	}

	resized();
}

void FloatingTabComponent::componentAdded(FloatingTile* newComponent)
{
	int i = getNumTabs();

	addTab(newComponent->getName(), Colours::transparentBlack, newComponent, false);

	getTabbedButtonBar().getTabButton(i)->setExtraComponent(new CloseButton(), TabBarButton::ExtraComponentPlacement::afterText);


	String text = newComponent->getCurrentFloatingPanel()->getCustomTitle();

	newComponent->addMouseListener(this, true);

	if (text.isEmpty())
		text = "Untitled";

	setTabName(i, text);

	setCurrentTabIndex(getNumTabs() - 1);

	notifySiblingChange();

	resized();
	repaint();
}

void FloatingTabComponent::componentRemoved(FloatingTile* deletedComponent)
{
	for (int i = 0; i < getNumTabs(); i++)
	{
		if (getTabContentComponent(i) == deletedComponent)
		{
			removeTab(i);
			break;
		}
	}

	deletedComponent->removeMouseListener(this);

	setCurrentTabIndex(getNumTabs() - 1);

	notifySiblingChange();

	resized();
	repaint();
}

void FloatingTabComponent::mouseDown(const MouseEvent& event)
{
	if (getNumTabs() == 1)
		return;

	int newTabIndex = getCurrentTabIndex();

	if (event.mods.isX2ButtonDown())
	{
		newTabIndex = jmin<int>(newTabIndex + 1, getNumTabs() - 1);

		if (newTabIndex != getCurrentTabIndex())
			setCurrentTabIndex(newTabIndex);
	}
	else if (event.mods.isX1ButtonDown())
	{
		newTabIndex = jmax<int>(newTabIndex - 1, 0);

		if (newTabIndex != getCurrentTabIndex())
			setCurrentTabIndex(newTabIndex);
	}
}

var FloatingTabComponent::toDynamicObject() const
{
	var obj = FloatingTileContainer::toDynamicObject();

	storePropertyInObject(obj, TabPropertyIds::CurrentTab, getCurrentTabIndex());

	return obj;
}

void FloatingTabComponent::fromDynamicObject(const var& objectData)
{
	clear();
	clearTabs();
	

	FloatingTileContainer::fromDynamicObject(objectData);

	setCurrentTabIndex(getPropertyWithDefault(objectData, TabPropertyIds::CurrentTab));
}

int FloatingTabComponent::getNumDefaultableProperties() const
{
	return TabPropertyIds::numTabPropertyIds;
}

Identifier FloatingTabComponent::getDefaultablePropertyId(int index) const
{
	if (index < FloatingTileContainer::ContainerPropertyIds::numContainerPropertyIds)
		return FloatingTileContainer::getDefaultablePropertyId(index);

	RETURN_DEFAULT_PROPERTY_ID(index, TabPropertyIds::CurrentTab, "CurrentTab");

	jassertfalse;
	return Identifier();
}

var FloatingTabComponent::getDefaultProperty(int id) const
{
	if (id < FloatingTileContainer::ContainerPropertyIds::numContainerPropertyIds)
		return FloatingTileContainer::getDefaultProperty(id);

	RETURN_DEFAULT_PROPERTY(id, TabPropertyIds::CurrentTab, -1);

	jassertfalse;

	return {};
}


void FloatingTabComponent::paint(Graphics& g)
{
	g.setColour(findPanelColour(PanelColourId::bgColour));
	g.fillRect(0, 0, getWidth(), 24);

	g.setColour(findPanelColour(PanelColourId::itemColour1));
	g.fillRect(0, 20, getWidth(), 4);
}

void FloatingTabComponent::resized()
{
	if (getParentComponent() == nullptr || getParentShell()->getCurrentFloatingPanel() == nullptr) // avoid resizing
		return;

	TabbedComponent::resized();

	if (getNumComponents() == getNumTabs())
	{
		for (int i = 0; i < getNumTabs(); i++)
		{
			String text = getComponent(i)->getCurrentFloatingPanel()->getBestTitle();

			if (text.isEmpty())
				text = "Untitled";

			setTabName(i, text);

			if (getComponent(i) != nullptr && !getComponent(i)->canBeDeleted())
			{
				getTabbedButtonBar().getTabButton(i)->setExtraComponent(nullptr, TabBarButton::afterText);
			}
		}
	}
	else
		jassertfalse;

	

	if (!isDynamic())
		addButton->setVisible(false);

	const int intend = FloatingTile::LayoutHelpers::showFoldButton(getParentShell()) ? 16 : 0;

	if (shouldIntendAddButton())
		getTabbedButtonBar().setTopLeftPosition(intend, 0);

	auto b = getTabbedButtonBar().getTabButton(getTabbedButtonBar().getNumTabs() - 1);

	if (b != nullptr)
		addButton->setBounds(b->getRight() + intend + 4, 2, 16, 16);
	else
		addButton->setBounds(intend + 2, 2, 16, 16);
}

void FloatingTabComponent::buttonClicked(Button* )
{
	addFloatingTile(new FloatingTile(getParentShell()->getMainController(), this));
}

void ResizableFloatingTileContainer::refreshLayout()
{
	FloatingTileContainer::refreshLayout();

	rebuildResizers();

	animate = false;
}

void ResizableFloatingTileContainer::paint(Graphics& g)
{
	g.setColour(findPanelColour(PanelColourId::bgColour));
	g.fillRect(getContainerBounds());
}

Rectangle<int> ResizableFloatingTileContainer::getContainerBounds() const
{
    auto localBounds = dynamic_cast<const Component*>(this)->getLocalBounds();
    
    return isTitleBarDisplayed() ? localBounds.withTrimmedTop(20) : localBounds;
}

int ResizableFloatingTileContainer::getMinimumOffset() const
{
	return getDimensionOffset(getContainerBounds());
}

int ResizableFloatingTileContainer::getMaximumOffset() const
{
	return getDimensionEnd(getContainerBounds());
}

int ResizableFloatingTileContainer::getDimensionSize(Rectangle<int> area) const
{
	return isVertical() ? area.getHeight() : area.getWidth();
}

int ResizableFloatingTileContainer::getDimensionOffset(Rectangle<int> area) const
{
	return isVertical() ? area.getY() : area.getX();
}

int ResizableFloatingTileContainer::getDimensionEnd(Rectangle<int> area) const
{
	return isVertical() ? area.getBottom() : area.getRight();
}

ResizableFloatingTileContainer::ResizableFloatingTileContainer(FloatingTile* parent, bool isVerticalTile) :
	FloatingTileContainer(parent),
	vertical(isVerticalTile)
{
	setDefaultPanelColour(PanelColourId::bgColour, Colour(0xff373737));
	setDefaultPanelColour(PanelColourId::itemColour1, HiseColourScheme::getColour(HiseColourScheme::ColourIds::EditorBackgroundColourIdBright));

	addAndMakeVisible(addButton = new ShapeButton("Add Column", Colours::white.withAlpha(0.7f), Colours::white, Colours::white));

	Path p;

	if (isVertical())
		p.loadPathFromData(ColumnIcons::addRowIcon, sizeof(ColumnIcons::addRowIcon));
	else

		p.loadPathFromData(ColumnIcons::addColumnIcon, sizeof(ColumnIcons::addColumnIcon));

	addButton->setShape(p, false, false, true);

	addButton->addListener(this);

	addFloatingTile(new FloatingTile(parent->getMainController(), this));

	setInterceptsMouseClicks(false, true);
}

ResizableFloatingTileContainer::~ResizableFloatingTileContainer()
{
	clear();
	currentlyDisplayedComponents.clear();
	addButton = nullptr;
	resizers.clear();
	
}

String ResizableFloatingTileContainer::getTitle() const
{
	return FloatingTileContent::getCustomTitle();
}

void ResizableFloatingTileContainer::buttonClicked(Button* b)
{
	if (b == addButton)
	{

		addFloatingTile(new FloatingTile(getParentShell()->getMainController(), this));

#if LAYOUT_OLD
		storeSizes();
#endif
	}
}

void ResizableFloatingTileContainer::resized()
{
	if (getParentComponent() == nullptr || getParentShell()->getCurrentFloatingPanel() == nullptr) // avoid resizing
		return;

	addButton->setVisible(isTitleBarDisplayed() && isDynamic());
	addButton->setBounds(shouldIntendAddButton() ? 18 : 1, 1, 30, 15);
	addButton->toFront(false);

	performLayout(getContainerBounds());
}

bool ResizableFloatingTileContainer::isTitleBarDisplayed() const
{
	return getParentShell()->showTitle();
}

void ResizableFloatingTileContainer::mouseDown(const MouseEvent& event)
{
	getParentShell()->mouseDown(event);
}

void ResizableFloatingTileContainer::componentAdded(FloatingTile* c)
{
	addAndMakeVisible(c);

	c->setBounds(0, 0, isVertical() ? 0 : getWidth(), isVertical() ? getHeight() : 0);

	notifySiblingChange();

	refreshLayout();
}

void ResizableFloatingTileContainer::componentRemoved(FloatingTile* c)
{
	removeChildComponent(c);

	notifySiblingChange();

	refreshLayout();
}

void ResizableFloatingTileContainer::performLayout(Rectangle<int> area)
{
	int visibleChildren = 0;

	for (int i = 0; i < getNumComponents(); i++)
	{
		if (getComponent(i)->getLayoutData().isVisible())
			visibleChildren++;
	}

	if (visibleChildren == 1)
	{
		for (int i = 0; i < resizers.size(); i++)
		{
			resizers[i]->setVisible(false);
		}

		for (int i = 0; i < getNumComponents(); i++)
		{
			auto c = getComponent(i);
			auto& lData = c->getLayoutData();


			c->setVisible(lData.isVisible());

			if (lData.isAbsolute())
			{
				int size = jmax<int>((int)lData.getCurrentSize(), 16);

				setBoundsOneDimension(c, getDimensionOffset(area), size, getContainerBounds());
			}
			else
			{
				
				c->setBounds(getContainerBounds());
			}
		}

		return;
	}

	int totalSize = getDimensionSize(area);

	// First pass: check all folded & fixed-size panels;

	int availableSize = totalSize;
	double totalRelativeAmount = 0.0;

	//int resizerWidth = getParentShell()->isLayoutModeEnabled() ? 8 : 4;

	for (int i = 0; i < getNumComponents(); i++)
	{
		auto pc = getComponent(i);
		auto& lData = pc->getLayoutData();

		pc->setVisible(lData.isVisible());
		
		if (!lData.isVisible())
			continue;

		if (i < getNumComponents() - 1)
			availableSize -= resizers[i]->getCurrentSize();

		if (pc->isFolded())
			availableSize -= 16;
		else if (lData.isAbsolute())
			availableSize -= (int)lData.getCurrentSize();
		else
			totalRelativeAmount += lData.getCurrentSize() * -1.0;
	}

	int offset = getDimensionOffset(area);
	int numToLayout = getNumComponents();

	for (int i = 0; i < numToLayout; i++)
	{
		auto pc = getComponent(i);
		auto& lData = pc->getLayoutData();

		if (i > 0)
		{
			auto resizer = resizers[i - 1];

			if (lData.isVisible())
			{
				auto rs = resizer->getCurrentSize();
				setBoundsOneDimension(resizer, offset, rs, area);
				offset += rs;
			}
			else
			{
				// It's visible by default so just hide it if necessary
				resizer->setVisible(false);
			}

			
		}

		if (!lData.isVisible())
			continue;

		if (pc->isFolded())
		{
			setBoundsOneDimension(pc, offset, 16, area);
			offset += 16;
		}
		else if (lData.isAbsolute())
		{
			int size = jmax<int>((int)lData.getCurrentSize(), 16);

			pc->setVisible(size > lData.getMinSize());

			setBoundsOneDimension(pc, offset, size, area);
			offset += (int)lData.getCurrentSize();
		}
		else
		{
			double percentage = lData.getCurrentSize() * -1.0;
			double scaledPercentage = percentage / totalRelativeAmount;
			int size = (int)(availableSize * scaledPercentage);
			size = jmax<int>(size, 16);

			pc->setVisible(size > lData.getMinSize());

			setBoundsOneDimension(pc, offset, size, area);
			offset += size;
		}
	}
}

void ResizableFloatingTileContainer::rebuildResizers()
{
	resizers.clear();

	if (getNumComponents() > 1)
	{
		for (int i = 1; i < getNumComponents(); i++)
		{
			resizers.add(new InternalResizer(this, i-1));
			addAndMakeVisible(resizers.getLast());

			resizers.getLast()->setVisible(resizers.getLast()->hasSomethingToDo());
		}
	}

	resized();
}

void ResizableFloatingTileContainer::setBoundsOneDimension(Component* c, int offset, int size, Rectangle<int> area)
{
	Rectangle<int> newBounds;

	if (isVertical())
	{
		newBounds = Rectangle<int>(area.getX(), offset, area.getWidth(), size);
	}
	else
	{
		newBounds = Rectangle<int>(offset, area.getY(), size, area.getHeight());
	}

	if (dynamic_cast<InternalResizer*>(c) == nullptr && animate)
	{
		if (c->isVisible())
			Desktop::getInstance().getAnimator().animateComponent(c, newBounds, 1.0f, 150, false, 1.3, 0.0);
		else
			c->setBounds(newBounds);
	}
	else
	{
		c->setBounds(newBounds);
	}
}

ResizableFloatingTileContainer::InternalResizer::InternalResizer(ResizableFloatingTileContainer* parent_, int index_) :
	parent(parent_),
	index(index_)
{
	int numTotalComponents = parent->getNumComponents();


	for (int i = 0; i < numTotalComponents; i++)
	{
		auto &lData = parent->getComponent(i)->getLayoutData();

		bool cantBeResized = parent->getComponent(i)->isFolded() || lData.isAbsolute();

		if (cantBeResized)
			continue;

		if (i <= index)
			prevPanels.add(parent->getComponent(i));
		else
			nextPanels.add(parent->getComponent(i));
	}


	setRepaintsOnMouseActivity(true);
	setMouseCursor(parent_->isVertical() ? MouseCursor::UpDownResizeCursor : MouseCursor::LeftRightResizeCursor);

	resizeIcon.loadPathFromData(ColumnIcons::bigResizeIcon, sizeof(ColumnIcons::bigResizeIcon));

	if (!parent->isVertical())
		resizeIcon.applyTransform(AffineTransform::rotation(float_Pi / 2.0f));
}


bool ResizableFloatingTileContainer::InternalResizer::hasSomethingToDo() const
{
	return prevPanels.size() != 0 && nextPanels.size() != 0;
}

int ResizableFloatingTileContainer::InternalResizer::getCurrentSize() const
{
	const bool isBig = parent->bigResizers[index];

	if (isBig)
		return 32;

	return parent->getParentShell()->isLayoutModeEnabled() ? 4 : 4;
}

void ResizableFloatingTileContainer::InternalResizer::paint(Graphics& g)
{
	g.setColour(parent->findPanelColour(ResizableFloatingTileContainer::PanelColourId::itemColour1));

	g.fillAll();

	Colour c = Colour(SIGNAL_COLOUR);

	if (active)
		c = c.withBrightness(0.8f);
	else if (isMouseOver())
		c = c.withAlpha(0.2f);
	else
		c = Colours::transparentBlack;

	g.fillAll(c);

	if (getWidth() > 17 && getHeight() > 17)
	{
		resizeIcon.scaleToFit((float)(getWidth() / 2 - 12), (float)(getHeight() / 2 - 12), 24.0f, 24.0f, true);
		g.setColour(Colours::white.withAlpha(active ? 1.0f : 0.2f));
		g.fillPath(resizeIcon);
	}

}

void ResizableFloatingTileContainer::InternalResizer::mouseDown(const MouseEvent& e)
{
	auto area = parent->getContainerBounds();

	auto e2 = e.getEventRelativeTo(parent);

	downOffset = parent->isVertical() ? e2.getMouseDownY() : e2.getMouseDownX();


	active = true;

	prevDownSizes.clear();
	nextDownSizes.clear();

	totalPrevDownSize = 0.0;

	for (auto prevPanel : prevPanels)
	{
		prevDownSizes.add(prevPanel->getLayoutData().getCurrentSize());
		totalPrevDownSize += prevDownSizes.getLast();
	}

	totalNextDownSize = 0.0;

	for (auto nextPanel : nextPanels)
	{
		nextDownSizes.add(nextPanel->getLayoutData().getCurrentSize());
		totalNextDownSize += nextDownSizes.getLast();
	}
}


void ResizableFloatingTileContainer::InternalResizer::mouseUp(const MouseEvent&)
{
	active = false;
}

void ResizableFloatingTileContainer::InternalResizer::mouseDrag(const MouseEvent& event)
{
	int delta = parent->isVertical() ? event.getDistanceFromDragStartY() : event.getDistanceFromDragStartX();
	auto area = parent->getContainerBounds();
	int totalSize = parent->isVertical() ? area.getHeight() : area.getWidth();
	int newWantedPosition = downOffset + delta;
	int actualPosition = jlimit<int>(parent->getMinimumOffset(), parent->getMaximumOffset(), newWantedPosition);
	int actualDelta = actualPosition - downOffset;
	double deltaRelative = (double)actualDelta / (double)totalSize;
	double prevNew = totalPrevDownSize - deltaRelative;
	double nextNew = totalNextDownSize + deltaRelative;
	double scalePrev = prevNew / totalPrevDownSize;
	double scaleNext = nextNew / totalNextDownSize;

	for (int i = 0; i < prevPanels.size(); i++)
	{
		auto s = jlimit<double>(-1.0, -0.001, scalePrev * prevDownSizes[i]);
		jassert(s < 0.0);
		prevPanels[i]->getLayoutData().setCurrentSize(s);
	}

	for (int i = 0; i < nextPanels.size(); i++)
	{
		auto s = jlimit<double>(-1.0, -0.001, scaleNext * nextDownSizes[i]);
		jassert(s < 0.0);
		nextPanels[i]->getLayoutData().setCurrentSize(s);
	}

	parent->resized();
}

} // namespace hise