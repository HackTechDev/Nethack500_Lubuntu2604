
local tut_ctrl_key = nil;
local tut_alt_key = nil;

function tut_key(command)
   local s = nh.eckey(command);
   local m = s:match("^^([A-Z])$"); -- ^X is Ctrl-X
   if (m ~= nil) then
      tut_ctrl_key = m;
      return "Ctrl-" .. m;
   end

   m = s:match("^M%-([A-Z])$"); -- M-X is Alt-X
   if (m ~= nil) then
      tut_alt_key = m;
      return "Alt-" .. m;
   end

   return s;
end

function tut_key_help(x, y)
   if (tut_ctrl_key ~= nil) then
      des.engraving({ coord = { x,y }, type = "engrave", text = tr("Note: Outside the tutorial, Ctrl-key combinations are shown prefixed with a caret, like '^%s'", tut_ctrl_key), degrade = false });
      tut_ctrl_key = nil;
   end
end

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noflip",
                "nomongen", "nodeathdrops", "noautosearch");

des.map([[
---------------------------------------------------------------------------
|-.--|.......|......|..S....|.F.......|.............|.......|.............|
|.-..........|......|--|....|.F.....|.|S-------.....|.....................|
||.--|.......|..T......|....|.F.....|.|.......|.....|.......|.............|
||.|.|.......|......|-.|....|.F.....|.|.......|.....|--------.............|
||.|.|.......|......||.|-.-----------.-.......|-S----.....................|
|-+-S---------..---.||........................|...|.......................|
|......|          |.-------------------.......|...|....--S----............|
|......|  ######  |.........|      |..S.......|...|....|.....|............|
|----.-| -+-   #  |.....---.|######+..|.......S...|....|.....|............|
|----+----.----+---.|.--|.|.|#     ------------...|....|.....F............|
|........|.|......|.|...F...|#  ........|.....+...|....|.....|............|
|.P......-S|......|------.---# .........|.....|...|....-------........----|
|..........|......+.|...|.|.S# ..--S-----.....|LLL|..................|..| |
|.W......---......|.|.|.|.|.|# ..|......|.....|LLL|..................|..--|
|....Z.L.S.F......|.|.|.|.---#   |......+.....|...|..................|..|.|
|........|--......|...|.....|####+......|.....|...+..................||...|
---------------------------------------------------------------------------
]]);


des.region(selection.area(01,01, 73, 16), "lit");

des.non_diggable();

des.teleport_region({ region = { 9,3, 9,3 } });

-- TODO:
--  - save (more of) hero state when entering
--  - quit-command should maybe exit the tutorial?

-- turn on some newbie-friendly options
nh.parse_config("OPTIONS=mention_walls");
nh.parse_config("OPTIONS=mention_decor");
nh.parse_config("OPTIONS=lit_corridor");

local movekeys = tut_key("movewest") .. " " ..
   tut_key("movesouth") .. " " ..
   tut_key("movenorth") .. " " ..
   tut_key("moveeast");

local diagmovekeys = tut_key("movesouthwest") .. " " ..
   tut_key("movenortheast") .. " " ..
   tut_key("movesoutheast") .. " " ..
   tut_key("movenorthwest");

des.engraving({ coord = { 9,3 }, type = "engrave", text = tr("Move around with %s", movekeys), degrade = false });
des.engraving({ coord = { 5,2 }, type = "engrave", text = tr("Move diagonally with %s", diagmovekeys), degrade = false });

if (u.role == "Knight") then
   des.engraving({ coord = { 12,1 }, type = "engrave", text = tr("Knights can jump with '%s'", tut_key("jump")), degrade = false });
end

--

des.engraving({ coord = { 2,4 }, type = "engrave", text = tr("Some actions may require multiple tries before succeeding"), degrade = false });
des.engraving({ coord = { 2,5 }, type = "engrave", text = tr("Open the door by moving into it"), degrade = false });
des.door({ coord = { 2,6 }, state = "closed" });

des.engraving({ coord = { 2,7 }, type = "engrave", text = tr("Close the door with '%s'", tut_key("close")), degrade = false });


--

des.engraving({ coord = { 4,5 }, type = "engrave", text = tr("You can leave the tutorial via the magic portal."), degrade = false });
des.trap({ type = "magic portal", coord = { 4,4 }, seen = true });

--

des.engraving({ coord = { 5,9 }, type = "engrave", text = tr("This door is locked. Kick it with '%s'", tut_key("kick")), degrade = false });
des.door({ coord = { 5,10 }, state = "locked" });

-- by default, kick is the first command that can be a ctrl-key combo
tut_key_help(6, 8);


des.engraving({ coord = { 5,12 }, type = "engrave", text = tr("Look around the map with '%s', press ESC when you're done", tut_key("glance")), degrade = false });

--

des.engraving({ coord = { 10,13 }, type = "engrave", text = tr("Use '%s' to search for secret doors", tut_key("search")), degrade = false });

des.engraving({ coord = { 10,15 }, type = "engrave", text = tr("Wrong secret"), degrade = false });

--

des.engraving({ coord = { 10,10 }, type = "engrave", text = tr("Behind this door is a dark corridor"), degrade = false });
des.door({ coord = { 10,9 }, state = percent(50) and "locked" or "closed" });
des.region(selection.match("#"), "unlit");
des.region(selection.match(" "), "unlit");
des.door({ coord = { 15,10 }, state = percent(50) and "locked" or "closed" });

--

des.engraving({ coord = { 15,11 }, type = "engrave", text = tr("There are four traps next to you! Search for them."), degrade = false });
local locs = { {14,11}, {14,12}, {15,12}, {16,12}, {16,11} };
shuffle(locs);
for i = 1, 4 do
   des.trap({ type = percent(50) and "sleep gas" or "board",
              coord = locs[i], victim = false });
end

des.engraving({ coord = { 15,15 }, type = "engrave", text = tr("Some traps can be disabled with '%s'", tut_key("untrap")), degrade = false });
des.trap({ coord = { 15,16 }, type = "web", spider_on_web = false });

--

des.door({ coord = { 18,13 }, state = "closed" });

des.engraving({ coord = { 19,13 }, type = "engrave", text = tr("Pick up items with '%s'", tut_key("pickup")), degrade = false });

local armor = (u.role == "Monk") and "leather gloves" or "leather armor";

des.object({ id = armor, spe = 0, buc = "cursed", coord = { 19,14} });

des.engraving({ coord = { 19,15 }, type = "engrave", text = tr("Wear armor with '%s'", tut_key("wear")), degrade = false });

des.object({ id = "dagger", spe = 0, buc = "not-cursed", coord = { 21,15} });

des.engraving({ coord = { 21,14 }, type = "engrave", text = tr("Wield weapons with '%s'", tut_key("wield")), degrade = false });


des.engraving({ coord = { 22,13 }, type = "engrave", text = tr("Hit monsters by walking into them."), degrade = false });

des.monster({ id = "lichen", coord = { 23,15 }, waiting = true, countbirth = false });

--

des.engraving({ coord = { 24,16 }, type = "engrave", text = tr("Now you know the very basics. You can leave the tutorial via the magic portal."), degrade = false });

des.engraving({ coord = { 26,16 }, type = "engrave", text = tr("Step into this portal to leave the tutorial"), degrade = false });
des.trap({ type = "magic portal", coord = { 27,16 }, seen = true });

--

des.engraving({ coord = { 25,13 }, type = "engrave", text = tr("Push boulders by moving into them"), degrade = false });
des.object({ id = "boulder", coord = {25,12} });

--

des.engraving({ coord = { 27,9 }, type = "engrave", text = tr("Take off armor with '%s'", tut_key("takeoff")), degrade = false });

--

des.object({ class = "?", id = "remove curse", buc = "blessed", coord = {23,11} })
des.engraving({ coord = { 22,11 }, type = "engrave", text = tr("Some items have shuffled descriptions, different each game"), degrade = false });
des.engraving({ coord = { 23,11 }, type = "engrave", text = tr("Pick up this scroll, read it with '%s', and try to remove the armor again", tut_key("read")), degrade = false });

--

des.engraving({ coord = { 19,10 }, type = "engrave", text = tr("Another magic portal, a way to leave this tutorial"), degrade = false });
des.trap({ type = "magic portal", coord = { 19,11 }, seen = true });

--

-- rock fall
des.object({ coord = {14, 5}, id = "rock", quantity = math.random(50,99) });
des.object({ coord = {15, 5}, id = "rock", quantity = math.random(10,30) });
des.object({ coord = {14, 4}, id = "rock", quantity = math.random(10,30) });
des.object({ coord = {15, 6}, id = "rock", quantity = math.random(30,60) });
des.object({ coord = {14, 6}, id = "rock", quantity = math.random(30,60) });
des.object({ coord = {14, 6}, id = "boulder" });

des.door({ coord = { 20,3 }, state = percent(50) and "open" or "closed" });

des.engraving({ coord = { 21,3 }, type = "engrave", text = tr("Avoid being burdened, it slows you down"), degrade = false });
des.engraving({ coord = { 22,3 }, type = "engrave", text = tr("Drop items with '%s'", tut_key("drop")), degrade = false });
des.engraving({ coord = { 22,4 }, type = "engrave", text = tr("You can drop partial stacks by prefixing the item slot letter with a number"), degrade = false });

--

des.monster({ id = "yellow mold", coord = { 26,2 }, waiting = true, countbirth = false });

des.engraving({ coord = { 25,5 }, type = "engrave", text = tr("Throw items with '%s'", tut_key("throw")), degrade = false });

des.trap({ type = "magic portal", coord = { 21,1 }, seen = true });

--

des.monster({ id = "wolf", coord = { 29,2 }, peaceful = 0, waiting = true, countbirth = false });

des.engraving({ coord = { 37,4 }, type = "engrave", text = tr("Missiles, such as rocks, work better when fired from appropriate launcher"), degrade = false });

des.object({ coord = { 37,3 }, id = "sling", buc = "not-cursed", spe = 9 });
des.engraving({ coord = { 37,3 }, type = "engrave", text = tr("Wield the sling"), degrade = false });
des.engraving({ coord = { 36,1 }, type = "engrave", text = tr("Use '%s' to fire missiles with the wielded launcher", tut_key("fire")), degrade = false });

des.engraving({ coord = { 35,4 }, type = "engrave", text = tr("Firing launches items from your quiver; Use '%s' to put items in it", tut_key("quiver")), degrade = false });

des.engraving({ coord = { 33,4 }, type = "engrave", text = tr("You can wait a turn with '%s'", tut_key("wait")), degrade = false });


--

des.door({ coord = { 38,6 }, state = "closed" });

des.engraving({ coord = { 39,6 }, type = "engrave", text = tr("You loot containers with '%s'", tut_key("loot")), degrade = false });

des.object({ coord = { 41,6 }, id = "large box", broken = true, trapped = false,
             contents = function(obj)
                des.object({ id = "secret door detection", class = "/", spe = 30 }); end
});
des.engraving({ coord = { 42,6 }, type = "engrave", text = tr("Containers can also be emptied with '%s'", tut_key("tip")), degrade = false });

des.engraving({ coord = { 45,6 }, type = "engrave", text = tr("Magic wands are used with '%s'", tut_key("zap")), degrade = false });

--

des.door({ coord = { 35,9 }, state = "nodoor" });
des.engraving({ coord = { 34,9 }, type = "engrave", text = tr("You can run by prefixing a movement key with '%s'", tut_key("run")), degrade = false });

--

des.door({ coord = { 33,16 }, state = "nodoor" });
des.engraving({ coord = { 35,15 }, type = "engrave", text = tr("Travel across the level with '%s'", tut_key("travel")), degrade = false });

--

des.trap({ type = "magic portal", coord = { 27,14 }, seen = true });

--

des.engraving({ coord = { 48,1 }, type = "burn", text = tr("Use '%s' to eat edible things", tut_key("eat")), degrade = false });

des.object({ coord = { 50,3 }, id = "apple", buc = "not-cursed"  });
des.object({ coord = { 50,3 }, id = "candy bar", buc = "not-cursed"  });

des.object({ coord = { 50,3 }, id = "corpse", montype = "lichen", buc = "not-cursed" });

--

des.door({ coord = { 46,11 }, state = "closed" });

des.engraving({ coord = { 43,11 }, type = "burn", text = tr("Use '%s' to use two weapons at once", tut_key("twoweapon")), degrade = false });
des.object({ coord = { 43,13 }, id = "knife", buc = "uncursed" });
des.object({ coord = { 43,14 }, id = "dagger", buc = "blessed" });

des.engraving({ coord = { 43,16 }, type = "burn", text = tr("Swap weapons quickly with '%s'", tut_key("swap")), degrade = false });

des.door({ coord = { 40,15 }, state = "random" });

--

des.object({ coord = { 48,7 }, id = "ring of levitation", buc = "not-cursed" });

des.engraving({ coord = { 48,10 }, type = "burn", text = tr("Put on accessories with '%s'", tut_key("puton")), degrade = false });

des.engraving({ coord = { 48,16 }, type = "burn", text = tr("Remove accessories with '%s'", tut_key("remove")), degrade = false });

des.door({ coord = { 50,16 }, state = "closed" });


--

des.engraving({ coord = { 58,9 }, type = "burn", text = tr("Use '%s' to go down the stairs", tut_key("down")), degrade = false });
des.stair({ dir = "down", coord = { 58,10 } });

--

-- one more ctrl-key help, if needed
tut_key_help(64, 4);

des.engraving({ coord = { 65,3 }, type = "burn", text = tr("UNDER CONSTRUCTION"), degrade = false });

des.trap({ type = "magic portal", coord = { 66,2 }, seen = true });

--

-- squeezing through small gaps

des.engraving({ coord = { 69,12 }, type = "burn", text = tr("Can't get through?  You're carrying too much."), degrade = false });

-- try to squeeze over boulders, find a trap door

des.object({ id = "boulder", coord = {71,16} });
des.object({ id = "boulder", coord = {72,16} });
des.object({ id = "boulder", coord = {73,16} });
des.trap({ type = "trap door", coord = { 73,15 } });

--

des.engraving({ coord = { 60,2 }, type = "engrave", text = tr("Spellcasting"), degrade = false });
if (u.uenmax < 5) then
   -- TODO: make sure hero has enough Pw to cast the spell (5 pw) instead?
   -- TODO: ensure the first cast of this spell succeeds?
   des.engraving({ coord = { 59,2 }, type = "engrave", text = tr("Unfortunately you don't have enough energy to cast spells."), degrade = false });
end
des.engraving({ coord = { 57,2 }, type = "engrave", text = tr("Pick up the spellbook with '%s'", tut_key("pickup")), degrade = false });
des.object({ coord = { 57,2 }, id = "spellbook of light", buc = "blessed" });
des.engraving({ coord = { 55,2 }, type = "engrave", text = tr("Read the spellbook with '%s'", tut_key("read")), degrade = false });
des.engraving({ coord = { 53,2 }, type = "engrave", text = tr("Use '%s' to cast a spell", tut_key("cast")), degrade = false });
des.region(selection.area(53,01, 59, 3), "unlit");

--

des.engraving({ coord = { 72,2 }, type = "engrave", text = tr("You \"quaff\" potions with '%s'", tut_key("quaff")), degrade = false });
des.object({ coord = { 72,2 }, id = "potion of object detection", buc = "blessed" });


----------------

-- entering and leaving tutorial _branch_ now handled by core
-- // nh.callback("cmd_before", "tutorial_cmd_before");
-- // nh.callback("level_enter", "tutorial_enter");
-- // nh.callback("level_leave", "tutorial_leave");
-- // nh.callback("end_turn", "tutorial_turn");

----------------

-- temporary stuff here
-- des.trap({ type = "magic portal", coord = { 9,5 }, seen = true });
-- des.trap({ type = "magic portal", coord = { 9,1 }, seen = true });
-- des.object({ id = "leather armor", spe = 0, coord = { 9,2} });

