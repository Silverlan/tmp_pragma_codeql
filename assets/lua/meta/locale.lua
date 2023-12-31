--- @meta
--- 
--- @class locale
locale = {}

--- 
--- @param id string
--- @param text string
function locale.set_text(id, text) end

--- 
--- @param identifier string
--- @param lan string
--- @param category string
--- @param text string
--- @return bool ret0
function locale.localize(identifier, lan, category, text) end

--- 
--- @return string ret0
function locale.get_language() end

--- 
--- @param lan string
function locale.change_language(lan) end

--- 
--- @param fileName string
--- @return bool ret0
function locale.load(fileName) end


