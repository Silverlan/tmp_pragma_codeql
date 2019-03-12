include("wicontextmenu.lua")

util.register_class("gui.WIMenuBar",gui.Base)

function gui.WIMenuBar:__init()
	gui.Base.__init(self)
end
function gui.WIMenuBar:OnRemove()

end
function gui.WIMenuBar:OnInitialize()
	gui.Base.OnInitialize(self)

	local pMain = gui.create("WIRect",self)
	pMain:SetColor(Color.Beige)
	pMain:SetAutoAlignToParent(true)
	pMain:Update()
	self.m_pMain = pMain

	self.m_tItems = {}
	local pClose = gui.create("WIButton",self)
	pClose:SetText("X")
	pClose:AddCallback("OnPressed",function(pClose)
		if(util.is_valid(self) == false) then return end
		self:CallCallbacks("OnClose")
	end)
	self.m_pClose = pClose
end
function gui.WIMenuBar:AddItem(name,fcContextCallback)
	local pItem = gui.create("WIMenuItem",self)
	if(util.is_valid(pItem) == false) then return end
	pItem:SetTitle(name)
	pItem:SetAction(function()
		if(gui.is_context_menu_open() == true) then return end
		local pContext = gui.open_context_menu()
		if(pContext ~= nil) then
			local pos = pItem:GetAbsolutePos()
			pos.y = pos.y +pItem:GetHeight()
			pContext:SetPos(pos)
			fcContextCallback(pContext)
		end
	end)
	table.insert(self.m_tItems,pItem)
	return pItem
end
function gui.WIMenuBar:Update()
	local x = 0
	local h = self:GetHeight()
	for _,pItem in ipairs(self.m_tItems) do
		if(util.is_valid(pItem) == true) then
			pItem:SizeToContents()
			pItem:SetX(x)
			pItem:SetHeight(h)
			x = x +pItem:GetWidth()
		end
	end
end
function gui.WIMenuBar:OnSizeChanged(w,h)
	if(util.is_valid(self.m_pClose) == true) then
		self.m_pClose:SetPos(w -h,0)
		self.m_pClose:SetSize(h,h)
	end
	self:Update()
end
gui.register("WIMenuBar",gui.WIMenuBar)
