/*
 * CCreatureHandler.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "CCreatureHandler.h"

#include "CGeneralTextHandler.h"
#include "filesystem/Filesystem.h"
#include "VCMI_Lib.h"
#include "CGameState.h"
#include "CTownHandler.h"
#include "CModHandler.h"
#include "GameSettings.h"
#include "StringConstants.h"
#include "serializer/JsonDeserializer.h"
#include "serializer/JsonUpdater.h"
#include "mapObjects/CObjectClassesHandler.h"

VCMI_LIB_NAMESPACE_BEGIN

const std::map<CCreature::CreatureQuantityId, std::string> CCreature::creatureQuantityRanges =
{
		{CCreature::CreatureQuantityId::FEW, "1-4"},
		{CCreature::CreatureQuantityId::SEVERAL, "5-9"},
		{CCreature::CreatureQuantityId::PACK, "10-19"},
		{CCreature::CreatureQuantityId::LOTS, "20-49"},
		{CCreature::CreatureQuantityId::HORDE, "50-99"},
		{CCreature::CreatureQuantityId::THRONG, "100-249"},
		{CCreature::CreatureQuantityId::SWARM, "250-499"},
		{CCreature::CreatureQuantityId::ZOUNDS, "500-999"},
		{CCreature::CreatureQuantityId::LEGION, "1000+"}
};

int32_t CCreature::getIndex() const
{
	return idNumber.toEnum();
}

int32_t CCreature::getIconIndex() const
{
	return iconIndex;
}

std::string CCreature::getJsonKey() const
{
	return modScope + ':' + identifier;
}

void CCreature::registerIcons(const IconRegistar & cb) const
{
	cb(getIconIndex(), 0, "CPRSMALL", smallIconName);
	cb(getIconIndex(), 0, "TWCRPORT", largeIconName);
}

CreatureID CCreature::getId() const
{
	return idNumber;
}

const IBonusBearer * CCreature::accessBonuses() const
{
	return this;
}

uint32_t CCreature::getMaxHealth() const
{
	return CBonusSystemNode::MaxHealth();
}

int32_t CCreature::getAdvMapAmountMin() const
{
	return ammMin;
}

int32_t CCreature::getAdvMapAmountMax() const
{
	return ammMax;
}

int32_t CCreature::getAIValue() const
{
	return AIValue;
}

int32_t CCreature::getFightValue() const
{
	return fightValue;
}

int32_t CCreature::getLevel() const
{
	return level;
}

int32_t CCreature::getGrowth() const
{
	return growth;
}

int32_t CCreature::getHorde() const
{
	return hordeGrowth;
}

int32_t CCreature::getFactionIndex() const
{
	return faction;
}

int32_t CCreature::getBaseAttack() const
{
	static const auto SELECTOR = Selector::typeSubtype(Bonus::PRIMARY_SKILL, PrimarySkill::ATTACK).And(Selector::sourceTypeSel(Bonus::CREATURE_ABILITY));
	return getExportedBonusList().valOfBonuses(SELECTOR);
}

int32_t CCreature::getBaseDefense() const
{
	static const auto SELECTOR = Selector::typeSubtype(Bonus::PRIMARY_SKILL, PrimarySkill::DEFENSE).And(Selector::sourceTypeSel(Bonus::CREATURE_ABILITY));
	return getExportedBonusList().valOfBonuses(SELECTOR);
}

int32_t CCreature::getBaseDamageMin() const
{
	static const auto SELECTOR = Selector::typeSubtype(Bonus::CREATURE_DAMAGE, 1).And(Selector::sourceTypeSel(Bonus::CREATURE_ABILITY));
	return getExportedBonusList().valOfBonuses(SELECTOR);
}

int32_t CCreature::getBaseDamageMax() const
{
	static const auto SELECTOR = Selector::typeSubtype(Bonus::CREATURE_DAMAGE, 2).And(Selector::sourceTypeSel(Bonus::CREATURE_ABILITY));
	return getExportedBonusList().valOfBonuses(SELECTOR);
}

int32_t CCreature::getBaseHitPoints() const
{
	static const auto SELECTOR = Selector::type()(Bonus::STACK_HEALTH).And(Selector::sourceTypeSel(Bonus::CREATURE_ABILITY));
	return getExportedBonusList().valOfBonuses(SELECTOR);
}

int32_t CCreature::getBaseSpellPoints() const
{
	static const auto SELECTOR = Selector::type()(Bonus::CASTS).And(Selector::sourceTypeSel(Bonus::CREATURE_ABILITY));
	return getExportedBonusList().valOfBonuses(SELECTOR);
}

int32_t CCreature::getBaseSpeed() const
{
	static const auto SELECTOR = Selector::type()(Bonus::STACKS_SPEED).And(Selector::sourceTypeSel(Bonus::CREATURE_ABILITY));
	return getExportedBonusList().valOfBonuses(SELECTOR);
}

int32_t CCreature::getBaseShots() const
{
	static const auto SELECTOR = Selector::type()(Bonus::SHOTS).And(Selector::sourceTypeSel(Bonus::CREATURE_ABILITY));
	return getExportedBonusList().valOfBonuses(SELECTOR);
}

int32_t CCreature::getCost(int32_t resIndex) const
{
	if(resIndex >= 0 && resIndex < cost.size())
		return cost[resIndex];
	else
		return 0;
}

std::string CCreature::getNameTranslated() const
{
	return getNameSingularTranslated();
}

std::string CCreature::getNamePluralTranslated() const
{
	return VLC->generaltexth->translate(getNamePluralTextID());
}

std::string CCreature::getNameSingularTranslated() const
{
	return VLC->generaltexth->translate(getNameSingularTextID());
}

std::string CCreature::getNameTextID() const
{
	return getNameSingularTextID();
}

std::string CCreature::getNamePluralTextID() const
{
	return TextIdentifier("creatures", modScope, identifier, "name", "plural" ).get();
}

std::string CCreature::getNameSingularTextID() const
{
	return TextIdentifier("creatures", modScope, identifier, "name", "singular" ).get();
}

CCreature::CreatureQuantityId CCreature::getQuantityID(const int & quantity)
{
	if (quantity<5)
		return CCreature::CreatureQuantityId::FEW;
	if (quantity<10)
		return CCreature::CreatureQuantityId::SEVERAL;
	if (quantity<20)
		return CCreature::CreatureQuantityId::PACK;
	if (quantity<50)
		return CCreature::CreatureQuantityId::LOTS;
	if (quantity<100)
		return CCreature::CreatureQuantityId::HORDE;
	if (quantity<250)
		return CCreature::CreatureQuantityId::THRONG;
	if (quantity<500)
		return CCreature::CreatureQuantityId::SWARM;
	if (quantity<1000)
		return CCreature::CreatureQuantityId::ZOUNDS;

	return CCreature::CreatureQuantityId::LEGION;
}

std::string CCreature::getQuantityRangeStringForId(const CCreature::CreatureQuantityId & quantityId)
{
	if(creatureQuantityRanges.find(quantityId) != creatureQuantityRanges.end())
		return creatureQuantityRanges.at(quantityId);

	logGlobal->error("Wrong quantityId: %d", (int)quantityId);
	assert(0);
	return "[ERROR]";
}

int CCreature::estimateCreatureCount(ui32 countID)
{
	static const int creature_count[] = { 0, 3, 8, 15, 35, 75, 175, 375, 750, 2500 };

	if(countID > 9)
	{
		logGlobal->error("Wrong countID %d!", countID);
		return 0;
	}
	else
		return creature_count[countID];
}

bool CCreature::isDoubleWide() const
{
	return doubleWide;
}

/**
 * Determines if the creature is of a good alignment.
 * @return true if the creture is good, false otherwise.
 */
bool CCreature::isGood () const
{
	return (*VLC->townh)[faction]->alignment == EAlignment::GOOD;
}

/**
 * Determines if the creature is of an evil alignment.
 * @return true if the creature is evil, false otherwise.
 */
bool CCreature::isEvil () const
{
	return (*VLC->townh)[faction]->alignment == EAlignment::EVIL;
}

si32 CCreature::maxAmount(const std::vector<si32> &res) const //how many creatures can be bought
{
	int ret = 2147483645;
	int resAmnt = static_cast<int>(std::min(res.size(),cost.size()));
	for(int i=0;i<resAmnt;i++)
		if(cost[i])
			ret = std::min(ret, (res[i] / cost[i]));
	return ret;
}

CCreature::CCreature()
{
	setNodeType(CBonusSystemNode::CREATURE);
	fightValue = AIValue = growth = hordeGrowth = ammMin = ammMax = 0;
}

void CCreature::addBonus(int val, Bonus::BonusType type, int subtype)
{
	auto selector = Selector::typeSubtype(type, subtype).And(Selector::source(Bonus::CREATURE_ABILITY, getIndex()));
	BonusList & exported = getExportedBonusList();

	BonusList existing;
	exported.getBonuses(existing, selector, Selector::all);

	if(existing.empty())
	{
		auto added = std::make_shared<Bonus>(Bonus::PERMANENT, type, Bonus::CREATURE_ABILITY, val, getIndex(), subtype, Bonus::BASE_NUMBER);
		addNewBonus(added);
	}
	else
	{
		std::shared_ptr<Bonus> b = existing[0];
		b->val = val;
	}
}

bool CCreature::isMyUpgrade(const CCreature *anotherCre) const
{
	//TODO upgrade of upgrade?
	return vstd::contains(upgrades, anotherCre->idNumber);
}

bool CCreature::valid() const
{
	return this == VLC->creh->objects[idNumber];
}

std::string CCreature::nodeName() const
{
	return "\"" + getNamePluralTextID() + "\"";
}

bool CCreature::isItNativeTerrain(TerrainId terrain) const
{
	auto native = getNativeTerrain();
	return native == terrain || native == ETerrainId::ANY_TERRAIN;
}

TerrainId CCreature::getNativeTerrain() const
{
	const std::string cachingStringNoTerrainPenalty = "type_NO_TERRAIN_PENALTY_sANY";
	static const auto selectorNoTerrainPenalty = Selector::typeSubtype(Bonus::NO_TERRAIN_PENALTY, static_cast<int>(ETerrainId::ANY_TERRAIN));

	//this code is used in the CreatureTerrainLimiter::limit to setup battle bonuses
	//and in the CGHeroInstance::getNativeTerrain() to setup movement bonuses or/and penalties.
	return hasBonus(selectorNoTerrainPenalty, cachingStringNoTerrainPenalty)
		? TerrainId(ETerrainId::ANY_TERRAIN)
		: (*VLC->townh)[faction]->nativeTerrain;
}

void CCreature::updateFrom(const JsonNode & data)
{
	JsonUpdater handler(nullptr, data);

	{
		auto configScope = handler.enterStruct("config");

		const JsonNode & configNode = handler.getCurrent();

		serializeJson(handler);

		if(!configNode["hitPoints"].isNull())
			addBonus(configNode["hitPoints"].Integer(), Bonus::STACK_HEALTH);

		if(!configNode["speed"].isNull())
			addBonus(configNode["speed"].Integer(), Bonus::STACKS_SPEED);

		if(!configNode["attack"].isNull())
			addBonus(configNode["attack"].Integer(), Bonus::PRIMARY_SKILL, PrimarySkill::ATTACK);

		if(!configNode["defense"].isNull())
			addBonus(configNode["defense"].Integer(), Bonus::PRIMARY_SKILL, PrimarySkill::DEFENSE);

		if(!configNode["damage"]["min"].isNull())
			addBonus(configNode["damage"]["min"].Integer(), Bonus::CREATURE_DAMAGE, 1);

		if(!configNode["damage"]["max"].isNull())
			addBonus(configNode["damage"]["max"].Integer(), Bonus::CREATURE_DAMAGE, 2);

		if(!configNode["shots"].isNull())
			addBonus(configNode["shots"].Integer(), Bonus::SHOTS);

		if(!configNode["spellPoints"].isNull())
			addBonus(configNode["spellPoints"].Integer(), Bonus::CASTS);
	}


	handler.serializeBonuses("bonuses", this);
}

void CCreature::serializeJson(JsonSerializeFormat & handler)
{
	handler.serializeInt("fightValue", fightValue);
	handler.serializeInt("aiValue", AIValue);
	handler.serializeInt("growth", growth);
	handler.serializeInt("horde", hordeGrowth);// Needed at least until configurable buildings

	{
		auto advMapNode = handler.enterStruct("advMapAmount");
		handler.serializeInt("min", ammMin);
		handler.serializeInt("max", ammMax);
	}

	if(handler.updating)
	{
		cost.serializeJson(handler, "cost");
		handler.serializeInt("faction", faction);//TODO: unify with deferred resolve
	}

	handler.serializeInt("level", level);
	handler.serializeBool("doubleWide", doubleWide);

	if(!handler.saving)
	{
		if(ammMin > ammMax)
			logMod->error("Invalid creature '%s' configuration, advMapAmount.min > advMapAmount.max", identifier);
	}
}

CCreatureHandler::CCreatureHandler()
	: expAfterUpgrade(0)
{
	VLC->creh = this;

	allCreatures.setDescription("All creatures");
	allCreatures.setNodeType(CBonusSystemNode::ENodeTypes::ALL_CREATURES);
	creaturesOfLevel[0].setDescription("Creatures of unnormalized tier");

	for(int i = 1; i < ARRAY_COUNT(creaturesOfLevel); i++)
		creaturesOfLevel[i].setDescription("Creatures of tier " + std::to_string(i));
	loadCommanders();
}

const CCreature * CCreatureHandler::getCreature(const std::string & scope, const std::string & identifier) const
{
	boost::optional<si32> index = VLC->modh->identifiers.getIdentifier(scope, "creature", identifier);

	if(!index)
		throw std::runtime_error("Creature not found "+identifier);

	return objects[*index];
}

void CCreatureHandler::loadCommanders()
{
	ResourceID configResource("config/commanders.json");

	std::string modSource = VLC->modh->findResourceOrigin(configResource);
	JsonNode data(configResource);
	data.setMeta(modSource);

	const JsonNode & config = data; // switch to const data accessors

	for (auto bonus : config["bonusPerLevel"].Vector())
	{
		commanderLevelPremy.push_back(JsonUtils::parseBonus(bonus.Vector()));
	}

	int i = 0;
	for (auto skill : config["skillLevels"].Vector())
	{
		skillLevels.emplace_back();
		for (auto skillLevel : skill["levels"].Vector())
		{
			skillLevels[i].push_back(static_cast<ui8>(skillLevel.Float()));
		}
		++i;
	}

	for (auto ability : config["abilityRequirements"].Vector())
	{
		std::pair <std::shared_ptr<Bonus>, std::pair <ui8, ui8> > a;
		a.first = JsonUtils::parseBonus (ability["ability"].Vector());
		a.second.first =  static_cast<ui8>(ability["skills"].Vector()[0].Float());
		a.second.second = static_cast<ui8>(ability["skills"].Vector()[1].Float());
		skillRequirements.push_back (a);
	}
}

void CCreatureHandler::loadBonuses(JsonNode & creature, std::string bonuses) const
{
	auto makeBonusNode = [&](const std::string & type, double val = 0) -> JsonNode
	{
		JsonNode ret;
		ret["type"].String() = type;
		ret["val"].Float() = val;
		return ret;
	};

	static const std::map<std::string, JsonNode> abilityMap =
	{
		{"FLYING_ARMY",            makeBonusNode("FLYING")},
		{"SHOOTING_ARMY",          makeBonusNode("SHOOTER")},
		{"SIEGE_WEAPON",           makeBonusNode("SIEGE_WEAPON")},
		{"const_free_attack",      makeBonusNode("BLOCKS_RETALIATION")},
		{"IS_UNDEAD",              makeBonusNode("UNDEAD")},
		{"const_no_melee_penalty", makeBonusNode("NO_MELEE_PENALTY")},
		{"const_jousting",         makeBonusNode("JOUSTING", 5)},
		{"KING_1",                 makeBonusNode("KING")}, // Slayer with no expertise
		{"KING_2",                 makeBonusNode("KING", 2)}, // Advanced Slayer or better
		{"KING_3",                 makeBonusNode("KING", 3)}, // Expert Slayer only
		{"const_no_wall_penalty",  makeBonusNode("NO_WALL_PENALTY")},
		{"MULTI_HEADED",           makeBonusNode("ATTACKS_ALL_ADJACENT")},
		{"IMMUNE_TO_MIND_SPELLS",  makeBonusNode("MIND_IMMUNITY")},
		{"HAS_EXTENDED_ATTACK",    makeBonusNode("TWO_HEX_ATTACK_BREATH")}
	};

	auto hasAbility = [&](const std::string & name) -> bool 
	{
		return boost::algorithm::find_first(bonuses, name);
	};

	for(const auto & a : abilityMap)
	{
		if(hasAbility(a.first))
			creature["abilities"][a.first] = a.second;
	}
	if(hasAbility("DOUBLE_WIDE"))
		creature["doubleWide"].Bool() = true;

	if(hasAbility("const_raises_morale"))
	{
		JsonNode node = makeBonusNode("MORALE");
		node["val"].Float() = 1;
		node["propagator"].String() = "HERO";
		creature["abilities"]["const_raises_morale"] = node;
	}
}

std::vector<JsonNode> CCreatureHandler::loadLegacyData()
{
	size_t dataSize = VLC->settings()->getInteger(EGameSettings::TEXTS_CREATURE);

	objects.resize(dataSize);
	std::vector<JsonNode> h3Data;
	h3Data.reserve(dataSize);

	CLegacyConfigParser parser("DATA/CRTRAITS.TXT");

	parser.endLine(); // header

	// this file is a bit different in some of Russian localisations:
	//ENG: Singular	Plural Wood ...
	//RUS: Singular	Plural	Plural2 Wood ...
	// Try to detect which version this is by header
	// TODO: use 3rd name? Stand for "whose", e.g. pikemans'
	size_t namesCount = 2;
	{
		if ( parser.readString() != "Singular" || parser.readString() != "Plural" )
			throw std::runtime_error("Incorrect format of CrTraits.txt");

		if (parser.readString() == "Plural2")
			namesCount = 3;

		parser.endLine();
	}

	for (size_t i=0; i<dataSize; i++)
	{
		//loop till non-empty line
		while (parser.isNextEntryEmpty())
			parser.endLine();

		JsonNode data;

		data["name"]["singular"].String() =  parser.readString();

		if (namesCount == 3)
			parser.readString();

		data["name"]["plural"].String() =  parser.readString();

		for(int v=0; v<7; ++v)
			data["cost"][GameConstants::RESOURCE_NAMES[v]].Float() = parser.readNumber();

		data["fightValue"].Float() = parser.readNumber();
		data["aiValue"].Float() = parser.readNumber();
		data["growth"].Float() = parser.readNumber();
		data["horde"].Float() = parser.readNumber();

		data["hitPoints"].Float() = parser.readNumber();
		data["speed"].Float() = parser.readNumber();
		data["attack"].Float() = parser.readNumber();
		data["defense"].Float() = parser.readNumber();
		data["damage"]["min"].Float() = parser.readNumber();
		data["damage"]["max"].Float() = parser.readNumber();

		if (float shots = parser.readNumber())
			data["shots"].Float() = shots;

		if (float spells = parser.readNumber())
			data["spellPoints"].Float() = spells;

		data["advMapAmount"]["min"].Float() = parser.readNumber();
		data["advMapAmount"]["max"].Float() = parser.readNumber();

		data["abilityText"].String() = parser.readString();
		loadBonuses(data, parser.readString()); //Attributes

		h3Data.push_back(data);
	}

	loadAnimationInfo(h3Data);

	return h3Data;
}

CCreature * CCreatureHandler::loadFromJson(const std::string & scope, const JsonNode & node, const std::string & identifier, size_t index)
{
	assert(identifier.find(':') == std::string::npos);
	assert(!scope.empty());

	auto * cre = new CCreature();

	if(node["hasDoubleWeek"].Bool())
	{
		doubledCreatures.insert(CreatureID(index));
	}
	cre->idNumber = CreatureID(index);
	cre->iconIndex = cre->getIndex() + 2;
	cre->identifier = identifier;
	cre->modScope = scope;

	JsonDeserializer handler(nullptr, node);
	cre->serializeJson(handler);

	cre->cost = Res::ResourceSet(node["cost"]);

	VLC->generaltexth->registerString(scope, cre->getNameSingularTextID(), node["name"]["singular"].String());
	VLC->generaltexth->registerString(scope, cre->getNamePluralTextID(), node["name"]["plural"].String());

	cre->addBonus(node["hitPoints"].Integer(), Bonus::STACK_HEALTH);
	cre->addBonus(node["speed"].Integer(), Bonus::STACKS_SPEED);
	cre->addBonus(node["attack"].Integer(), Bonus::PRIMARY_SKILL, PrimarySkill::ATTACK);
	cre->addBonus(node["defense"].Integer(), Bonus::PRIMARY_SKILL, PrimarySkill::DEFENSE);

	cre->addBonus(node["damage"]["min"].Integer(), Bonus::CREATURE_DAMAGE, 1);
	cre->addBonus(node["damage"]["max"].Integer(), Bonus::CREATURE_DAMAGE, 2);

	assert(node["damage"]["min"].Integer() <= node["damage"]["max"].Integer());

	if(!node["shots"].isNull())
		cre->addBonus(node["shots"].Integer(), Bonus::SHOTS);

	loadStackExperience(cre, node["stackExperience"]);
	loadJsonAnimation(cre, node["graphics"]);
	loadCreatureJson(cre, node);

	for(const auto & extraName : node["extraNames"].Vector())
	{
		for(const auto & type_name : getTypeNames())
			registerObject(scope, type_name, extraName.String(), cre->getIndex());
	}

	VLC->modh->identifiers.requestIdentifier(scope, "object", "monster", [=](si32 index)
	{
		JsonNode conf;
		conf.setMeta(scope);

		VLC->objtypeh->loadSubObject(cre->identifier, conf, Obj::MONSTER, cre->idNumber.num);
		if (!cre->advMapDef.empty())
		{
			JsonNode templ;
			templ["animation"].String() = cre->advMapDef;
			templ.setMeta(scope);
			VLC->objtypeh->getHandlerFor(Obj::MONSTER, cre->idNumber.num)->addTemplate(templ);
		}

		// object does not have any templates - this is not usable object (e.g. pseudo-creature like Arrow Tower)
		if (VLC->objtypeh->getHandlerFor(Obj::MONSTER, cre->idNumber.num)->getTemplates().empty())
			VLC->objtypeh->removeSubObject(Obj::MONSTER, cre->idNumber.num);
	});

	return cre;
}

const std::vector<std::string> & CCreatureHandler::getTypeNames() const
{
	static const std::vector<std::string> typeNames = { "creature" };
	return typeNames;
}

std::vector<bool> CCreatureHandler::getDefaultAllowed() const
{
	std::vector<bool> ret;

	ret.reserve(objects.size());
	for(const CCreature * crea : objects)
	{
		ret.push_back(crea ? !crea->special : false);
	}
	return ret;
}

void CCreatureHandler::loadCrExpBon()
{
	if (VLC->settings()->getBoolean(EGameSettings::MODULE_STACK_EXPERIENCE)) 	//reading default stack experience bonuses
	{
		CLegacyConfigParser parser("DATA/CREXPBON.TXT");

		Bonus b; //prototype with some default properties
		b.source = Bonus::STACK_EXPERIENCE;
		b.duration = Bonus::PERMANENT;
		b.valType = Bonus::ADDITIVE_VALUE;
		b.effectRange = Bonus::NO_LIMIT;
		b.additionalInfo = 0;
		b.turnsRemain = 0;
		BonusList bl;

		parser.endLine();

		parser.readString(); //ignore index
		loadStackExp(b, bl, parser);
		for(const auto & b : bl)
			addBonusForAllCreatures(b); //health bonus is common for all
		parser.endLine();

		for (int i = 1; i < 7; ++i)
		{
			for (int j = 0; j < 4; ++j) //four modifiers common for tiers
			{
				parser.readString(); //ignore index
				bl.clear();
				loadStackExp(b, bl, parser);
				for(const auto & b : bl)
					addBonusForTier(i, b);
				parser.endLine();
			}
		}
		for (int j = 0; j < 4; ++j) //tier 7
		{
			parser.readString(); //ignore index
			bl.clear();
			loadStackExp(b, bl, parser);
			for(const auto & b : bl)
			{
				addBonusForTier(7, b);
				creaturesOfLevel[0].addNewBonus(b); //bonuses from level 7 are given to high-level creatures
			}
			parser.endLine();
		}
		do //parse everything that's left
		{
			auto sid = static_cast<ui32>(parser.readNumber()); //id = this particular creature ID

			b.sid = sid;
			bl.clear();
			loadStackExp(b, bl, parser);
			for(const auto & b : bl)
			{
				objects[sid]->addNewBonus(b); //add directly to CCreature Node
			}
		}
		while (parser.endLine());

		//Calculate rank exp values, formula appears complicated bu no parsing needed
		expRanks.resize(8);
		int dif = 0;
		int it = 8000; //ignore name of this variable
		expRanks[0].push_back(it);
		for (int j = 1; j < 10; ++j) //used for tiers 8-10, and all other probably
		{
			expRanks[0].push_back(expRanks[0][j-1] + it + dif);
			dif += it/5;
		}
		for (int i = 1; i < 8; ++i)
		{
			dif = 0;
			it = 1000 * i;
			expRanks[i].push_back(it);
			for (int j = 1; j < 10; ++j)
			{
				expRanks[i].push_back(expRanks[i][j-1] + it + dif);
				dif += it/5;
			}
		}

		CLegacyConfigParser expBonParser("DATA/CREXPMOD.TXT");

		expBonParser.endLine(); //header

		maxExpPerBattle.resize(8);
		for (int i = 1; i < 8; ++i)
		{
			expBonParser.readString(); //index
			expBonParser.readString(); //float multiplier -> hardcoded
			expBonParser.readString(); //ignore upgrade mod? ->hardcoded
			expBonParser.readString(); //already calculated

			maxExpPerBattle[i] = static_cast<ui32>(expBonParser.readNumber());
			expRanks[i].push_back(expRanks[i].back() + static_cast<ui32>(expBonParser.readNumber()));

			expBonParser.endLine();
		}
		//skeleton gets exp penalty
		objects[56].get()->addBonus(-50, Bonus::EXP_MULTIPLIER, -1);
		objects[57].get()->addBonus(-50, Bonus::EXP_MULTIPLIER, -1);
		//exp for tier >7, rank 11
		expRanks[0].push_back(147000);
		expAfterUpgrade = 75; //percent
		maxExpPerBattle[0] = maxExpPerBattle[7];

	}//end of Stack Experience
}

void CCreatureHandler::loadAnimationInfo(std::vector<JsonNode> &h3Data) const
{
	CLegacyConfigParser parser("DATA/CRANIM.TXT");

	parser.endLine(); // header
	parser.endLine();

	for(int dd = 0; dd < VLC->settings()->getInteger(EGameSettings::TEXTS_CREATURE); ++dd)
	{
		while (parser.isNextEntryEmpty() && parser.endLine()) // skip empty lines
			;

		loadUnitAnimInfo(h3Data[dd]["graphics"], parser);
		parser.endLine();
	}
}

void CCreatureHandler::loadUnitAnimInfo(JsonNode & graphics, CLegacyConfigParser & parser) const
{
	graphics["timeBetweenFidgets"].Float() = parser.readNumber();

	JsonNode & animationTime = graphics["animationTime"];
	animationTime["walk"].Float() = parser.readNumber();
	animationTime["attack"].Float() = parser.readNumber();
	parser.readNumber(); // unused value "Flight animation time" - H3 actually uses "Walk animation time" even for flying creatures
	animationTime["idle"].Float() = 10.0;

	JsonNode & missile = graphics["missile"];
	JsonNode & offsets = missile["offset"];

	offsets["upperX"].Float() = parser.readNumber();
	offsets["upperY"].Float() = parser.readNumber();
	offsets["middleX"].Float() = parser.readNumber();
	offsets["middleY"].Float() = parser.readNumber();
	offsets["lowerX"].Float() = parser.readNumber();
	offsets["lowerY"].Float() = parser.readNumber();

	for(int i=0; i<12; i++)
	{
		JsonNode entry;
		entry.Float() = parser.readNumber();
		missile["frameAngles"].Vector().push_back(entry);
	}

	graphics["troopCountLocationOffset"].Float() = parser.readNumber();

	missile["attackClimaxFrame"].Float() = parser.readNumber();

	// assume that creature is not a shooter and should not have whole missile field
	if (missile["frameAngles"].Vector()[0].Float() == 0 &&
	    missile["attackClimaxFrame"].Float() == 0)
		graphics.Struct().erase("missile");
}

void CCreatureHandler::loadJsonAnimation(CCreature * cre, const JsonNode & graphics) const
{
	cre->animation.timeBetweenFidgets = graphics["timeBetweenFidgets"].Float();
	cre->animation.troopCountLocationOffset = static_cast<int>(graphics["troopCountLocationOffset"].Float());

	const JsonNode & animationTime = graphics["animationTime"];
	cre->animation.walkAnimationTime = animationTime["walk"].Float();
	cre->animation.idleAnimationTime = animationTime["idle"].Float();
	cre->animation.attackAnimationTime = animationTime["attack"].Float();

	const JsonNode & missile = graphics["missile"];
	const JsonNode & offsets = missile["offset"];
	cre->animation.upperRightMissleOffsetX = static_cast<int>(offsets["upperX"].Float());
	cre->animation.upperRightMissleOffsetY = static_cast<int>(offsets["upperY"].Float());
	cre->animation.rightMissleOffsetX =      static_cast<int>(offsets["middleX"].Float());
	cre->animation.rightMissleOffsetY =      static_cast<int>(offsets["middleY"].Float());
	cre->animation.lowerRightMissleOffsetX = static_cast<int>(offsets["lowerX"].Float());
	cre->animation.lowerRightMissleOffsetY = static_cast<int>(offsets["lowerY"].Float());

	cre->animation.attackClimaxFrame = static_cast<int>(missile["attackClimaxFrame"].Float());
	cre->animation.missleFrameAngles = missile["frameAngles"].convertTo<std::vector<double> >();

	cre->advMapDef = graphics["map"].String();
	cre->smallIconName = graphics["iconSmall"].String();
	cre->largeIconName = graphics["iconLarge"].String();
}

void CCreatureHandler::loadCreatureJson(CCreature * creature, const JsonNode & config) const
{
	creature->animDefName = config["graphics"]["animation"].String();

	//FIXME: MOD COMPATIBILITY
	if (config["abilities"].getType() == JsonNode::JsonType::DATA_STRUCT)
	{
		for(const auto & ability : config["abilities"].Struct())
		{
			if (!ability.second.isNull())
			{
				auto b = JsonUtils::parseBonus(ability.second);
				b->source = Bonus::CREATURE_ABILITY;
				b->sid = creature->getIndex();
				b->duration = Bonus::PERMANENT;
				creature->addNewBonus(b);
			}
		}
	}
	else
	{
		for(const JsonNode &ability : config["abilities"].Vector())
		{
			if(ability.getType() == JsonNode::JsonType::DATA_VECTOR)
			{
				logMod->error("Ignored outdated creature ability format in %s", creature->getJsonKey());
			}
			else
			{
				auto b = JsonUtils::parseBonus(ability);
				b->source = Bonus::CREATURE_ABILITY;
				b->sid = creature->getIndex();
				b->duration = Bonus::PERMANENT;
				creature->addNewBonus(b);
			}
		}
	}

	VLC->modh->identifiers.requestIdentifier("faction", config["faction"], [=](si32 faction)
	{
		creature->faction = faction;
	});

	for(const JsonNode &value : config["upgrades"].Vector())
	{
		VLC->modh->identifiers.requestIdentifier("creature", value, [=](si32 identifier)
		{
			creature->upgrades.insert(CreatureID(identifier));
		});
	}

	creature->animation.projectileImageName = config["graphics"]["missile"]["projectile"].String();

	for(const JsonNode & value : config["graphics"]["missile"]["ray"].Vector())
	{
		CCreature::CreatureAnimation::RayColor color;

		color.start.r = value["start"].Vector()[0].Integer();
		color.start.g = value["start"].Vector()[1].Integer();
		color.start.b = value["start"].Vector()[2].Integer();
		color.start.a = value["start"].Vector()[3].Integer();

		color.end.r = value["end"].Vector()[0].Integer();
		color.end.g = value["end"].Vector()[1].Integer();
		color.end.b = value["end"].Vector()[2].Integer();
		color.end.a = value["end"].Vector()[3].Integer();

		creature->animation.projectileRay.push_back(color);
	}

	creature->special = config["special"].Bool() || config["disabled"].Bool();

	const JsonNode & sounds = config["sound"];

#define GET_SOUND_VALUE(value_name) creature->sounds.value_name = sounds[#value_name].String()
	GET_SOUND_VALUE(attack);
	GET_SOUND_VALUE(defend);
	GET_SOUND_VALUE(killed);
	GET_SOUND_VALUE(move);
	GET_SOUND_VALUE(shoot);
	GET_SOUND_VALUE(wince);
	GET_SOUND_VALUE(startMoving);
	GET_SOUND_VALUE(endMoving);
#undef GET_SOUND_VALUE
}

void CCreatureHandler::loadStackExperience(CCreature * creature, const JsonNode & input) const
{
	for (const JsonNode &exp : input.Vector())
	{
		const JsonVector &values = exp["values"].Vector();
		int lowerLimit = 1;//, upperLimit = 255;
		if (values[0].getType() == JsonNode::JsonType::DATA_BOOL)
		{
			for (const JsonNode &val : values)
			{
				if(val.Bool())
				{
					// parse each bonus separately
					// we can not create copies since identifiers resolution does not tracks copies
					// leading to unset identifier values in copies
					auto bonus = JsonUtils::parseBonus (exp["bonus"]);
					bonus->source = Bonus::STACK_EXPERIENCE;
					bonus->duration = Bonus::PERMANENT;
					bonus->limiter = std::make_shared<RankRangeLimiter>(RankRangeLimiter(lowerLimit));
					creature->addNewBonus (bonus);
					break; //TODO: allow bonuses to turn off?
				}
				++lowerLimit;
			}
		}
		else
		{
			int lastVal = 0;
			for (const JsonNode &val : values)
			{
				if (val.Float() != lastVal)
				{
					JsonNode bonusInput = exp["bonus"];
					bonusInput["val"].Float() = static_cast<int>(val.Float()) - lastVal;

					auto bonus = JsonUtils::parseBonus (bonusInput);
					bonus->source = Bonus::STACK_EXPERIENCE;
					bonus->duration = Bonus::PERMANENT;
					bonus->limiter.reset (new RankRangeLimiter(lowerLimit));
					creature->addNewBonus (bonus);
				}
				lastVal = static_cast<int>(val.Float());
				++lowerLimit;
			}
		}
	}
}

void CCreatureHandler::loadStackExp(Bonus & b, BonusList & bl, CLegacyConfigParser & parser) const//help function for parsing CREXPBON.txt
{
	bool enable = false; //some bonuses are activated with values 2 or 1
	std::string buf = parser.readString();
	std::string mod = parser.readString();

	switch (buf[0])
	{
	case 'H':
		b.type = Bonus::STACK_HEALTH;
		b.valType = Bonus::PERCENT_TO_BASE;
		break;
	case 'A':
		b.type = Bonus::PRIMARY_SKILL;
		b.subtype = PrimarySkill::ATTACK;
		break;
	case 'D':
		b.type = Bonus::PRIMARY_SKILL;
		b.subtype = PrimarySkill::DEFENSE;
		break;
	case 'M': //Max damage
		b.type = Bonus::CREATURE_DAMAGE;
		b.subtype = 2;
		break;
	case 'm': //Min damage
		b.type = Bonus::CREATURE_DAMAGE;
		b.subtype = 1;
		break;
	case 'S':
		b.type = Bonus::STACKS_SPEED; break;
	case 'O':
		b.type = Bonus::SHOTS; break;
	case 'b':
		b.type = Bonus::ENEMY_DEFENCE_REDUCTION; break;
	case 'C':
		b.type = Bonus::CHANGES_SPELL_COST_FOR_ALLY; break;
	case 'd':
		b.type = Bonus::DEFENSIVE_STANCE; break;
	case 'e':
		b.type = Bonus::DOUBLE_DAMAGE_CHANCE;
		b.subtype = 0;
		break;
	case 'E':
		b.type = Bonus::DEATH_STARE;
		b.subtype = 0; //Gorgon
		break;
	case 'F':
		b.type = Bonus::FEAR; break;
	case 'g':
		b.type = Bonus::SPELL_DAMAGE_REDUCTION;
		b.subtype = -1; //all magic schools
		break;
	case 'P':
		b.type = Bonus::CASTS; break;
	case 'R':
		b.type = Bonus::ADDITIONAL_RETALIATION; break;
	case 'W':
		b.type = Bonus::MAGIC_RESISTANCE;
		b.subtype = 0; //otherwise creature window goes crazy
		break;
	case 'f': //on-off skill
		enable = true; //sometimes format is: 2 -> 0, 1 -> 1
		switch (mod[0])
		{
			case 'A':
				b.type = Bonus::ATTACKS_ALL_ADJACENT; break;
			case 'b':
				b.type = Bonus::RETURN_AFTER_STRIKE; break;
			case 'B':
				b.type = Bonus::TWO_HEX_ATTACK_BREATH; break;
			case 'c':
				b.type = Bonus::JOUSTING; 
				b.val = 5;
				break;
			case 'D':
				b.type = Bonus::ADDITIONAL_ATTACK; break;
			case 'f':
				b.type = Bonus::FEARLESS; break;
			case 'F':
				b.type = Bonus::FLYING; break;
			case 'm':
				b.type = Bonus::MORALE; break;
				b.val = 1;
				b.valType = Bonus::INDEPENDENT_MAX;
				break;
			case 'M':
				b.type = Bonus::NO_MORALE; break;
			case 'p': //Mind spells
			case 'P':
				b.type = Bonus::MIND_IMMUNITY; break;
			case 'r':
				b.type = Bonus::REBIRTH; //on/off? makes sense?
				b.subtype = 0;
				b.val = 20; //arbitrary value
				break;
			case 'R':
				b.type = Bonus::BLOCKS_RETALIATION; break;
			case 's':
				b.type = Bonus::FREE_SHOOTING; break;
			case 'u':
				b.type = Bonus::SPELL_RESISTANCE_AURA; break;
			case 'U':
				b.type = Bonus::UNDEAD; break;
			default:
				logGlobal->trace("Not parsed bonus %s %s", buf, mod);
				return;
				break;
		}
		break;
	case 'w': //specific spell immunities, enabled/disabled
		enable = true;
		switch (mod[0])
		{
			case 'B': //Blind
				b.type = Bonus::SPELL_IMMUNITY;
				b.subtype = SpellID::BLIND;
				b.additionalInfo = 0;//normal immunity
				break;
			case 'H': //Hypnotize
				b.type = Bonus::SPELL_IMMUNITY;
				b.subtype = SpellID::HYPNOTIZE;
				b.additionalInfo = 0;//normal immunity
				break;
			case 'I': //Implosion
				b.type = Bonus::SPELL_IMMUNITY;
				b.subtype = SpellID::IMPLOSION;
				b.additionalInfo = 0;//normal immunity
				break;
			case 'K': //Berserk
				b.type = Bonus::SPELL_IMMUNITY;
				b.subtype = SpellID::BERSERK;
				b.additionalInfo = 0;//normal immunity
				break;
			case 'M': //Meteor Shower
				b.type = Bonus::SPELL_IMMUNITY;
				b.subtype = SpellID::METEOR_SHOWER;
				b.additionalInfo = 0;//normal immunity
				break;
			case 'N': //dispell beneficial spells
				b.type = Bonus::SPELL_IMMUNITY;
				b.subtype = SpellID::DISPEL_HELPFUL_SPELLS;
				b.additionalInfo = 0;//normal immunity
				break;
			case 'R': //Armageddon
				b.type = Bonus::SPELL_IMMUNITY;
				b.subtype = SpellID::ARMAGEDDON;
				b.additionalInfo = 0;//normal immunity
				break;
			case 'S': //Slow
				b.type = Bonus::SPELL_IMMUNITY;
				b.subtype = SpellID::SLOW;
				b.additionalInfo = 0;//normal immunity
				break;
			case '6':
			case '7':
			case '8':
			case '9':
				b.type = Bonus::LEVEL_SPELL_IMMUNITY;
				b.val = std::atoi(mod.c_str()) - 5;
				break;
			case ':':
				b.type = Bonus::LEVEL_SPELL_IMMUNITY;
				b.val = GameConstants::SPELL_LEVELS; //in case someone adds higher level spells?
				break;
			case 'F':
				b.type = Bonus::FIRE_IMMUNITY;
				b.subtype = 1; //not positive
				break;
			case 'O':
				b.type = Bonus::FIRE_IMMUNITY;
				b.subtype = 2; //only direct damage
				break;
			case 'f':
				b.type = Bonus::FIRE_IMMUNITY;
				b.subtype = 0; //all
				break;
			case 'C':
				b.type = Bonus::WATER_IMMUNITY;
				b.subtype = 1; //not positive
				break;
			case 'W':
				b.type = Bonus::WATER_IMMUNITY;
				b.subtype = 2; //only direct damage
				break;
			case 'w':
				b.type = Bonus::WATER_IMMUNITY;
				b.subtype = 0; //all
				break;
			case 'E':
				b.type = Bonus::EARTH_IMMUNITY;
				b.subtype = 2; //only direct damage
				break;
			case 'e':
				b.type = Bonus::EARTH_IMMUNITY;
				b.subtype = 0; //all
				break;
			case 'A':
				b.type = Bonus::AIR_IMMUNITY;
				b.subtype = 2; //only direct damage
				break;
			case 'a':
				b.type = Bonus::AIR_IMMUNITY;
				b.subtype = 0; //all
				break;
			case 'D':
				b.type = Bonus::DIRECT_DAMAGE_IMMUNITY;
				break;
			case '0':
				b.type = Bonus::RECEPTIVE;
				break;
			case 'm':
				b.type = Bonus::MIND_IMMUNITY;
				break;
			default:
				logGlobal->trace("Not parsed bonus %s %s", buf, mod);
				return;
		}
		break;

	case 'i':
		enable = true;
		b.type = Bonus::NO_DISTANCE_PENALTY;
		break;
	case 'o':
		enable = true;
		b.type = Bonus::NO_WALL_PENALTY;
		break;

	case 'a':
	case 'c':
	case 'K':
	case 'k':
		b.type = Bonus::SPELL_AFTER_ATTACK;
		b.subtype = stringToNumber(mod);
		break;
	case 'h':
		b.type= Bonus::HATE;
		b.subtype = stringToNumber(mod);
		break;
	case 'p':
	case 'J':
		b.type = Bonus::SPELL_BEFORE_ATTACK;
		b.subtype = stringToNumber(mod);
		b.additionalInfo = 3; //always expert?
		break;
	case 'r':
		b.type = Bonus::HP_REGENERATION;
		b.val = stringToNumber(mod);
		break;
	case 's':
		b.type = Bonus::ENCHANTED;
		b.subtype = stringToNumber(mod);
		b.valType = Bonus::INDEPENDENT_MAX;
		break;
	default:
		logGlobal->trace("Not parsed bonus %s %s", buf, mod);
		return;
		break;
	}
	switch (mod[0])
	{
		case '+':
		case '=': //should we allow percent values to stack or pick highest?
			b.valType = Bonus::ADDITIVE_VALUE;
			break;
	}

	//limiters, range
	si32 lastVal;
	si32 curVal;
	si32 lastLev = 0;

	if (enable) //0 and 2 means non-active, 1 - active
	{
		if (b.type != Bonus::REBIRTH)
			b.val = 0; //on-off ability, no value specified
		parser.readNumber(); // 0 level is never active
		for (int i = 1; i < 11; ++i)
		{
			curVal = static_cast<si32>(parser.readNumber());
			if (curVal == 1)
			{
				b.limiter.reset (new RankRangeLimiter(i));
				bl.push_back(std::make_shared<Bonus>(b));
				break; //never turned off it seems
			}
		}
	}
	else
	{
		lastVal = static_cast<si32>(parser.readNumber());
		if (b.type == Bonus::HATE)
			lastVal *= 10; //odd fix
		//FIXME: value for zero level should be stored in our config files (independent of stack exp)
		for (int i = 1; i < 11; ++i)
		{
			curVal = static_cast<si32>(parser.readNumber());
			if (b.type == Bonus::HATE)
				curVal *= 10; //odd fix
			if (curVal > lastVal) //threshold, add new bonus
			{
				b.val = curVal - lastVal;
				lastVal = curVal;
				b.limiter.reset (new RankRangeLimiter(i));
				bl.push_back(std::make_shared<Bonus>(b));
				lastLev = i; //start new range from here, i = previous rank
			}
			else if (curVal < lastVal)
			{
				b.val = lastVal;
				b.limiter.reset (new RankRangeLimiter(lastLev, i));
			}
		}
	}
}

int CCreatureHandler::stringToNumber(std::string & s) const
{
	boost::algorithm::replace_first(s,"#",""); //drop hash character
	return std::atoi(s.c_str());
}

CCreatureHandler::~CCreatureHandler()
{
	for(auto & p : skillRequirements)
		p.first = nullptr;
}

CreatureID CCreatureHandler::pickRandomMonster(CRandomGenerator & rand, int tier) const
{
	int r = 0;
	if(tier == -1) //pick any allowed creature
	{
		do
		{
			r = (*RandomGeneratorUtil::nextItem(objects, rand))->idNumber;
		} while (objects[r] && objects[r]->special); // find first "not special" creature
	}
	else
	{
		assert(vstd::iswithin(tier, 1, 7));
		std::vector<CreatureID> allowed;
		for(const CBonusSystemNode *b : creaturesOfLevel[tier].getChildrenNodes())
		{
			assert(b->getNodeType() == CBonusSystemNode::CREATURE);
			const auto * crea = dynamic_cast<const CCreature *>(b);
			if(crea && !crea->special)
				allowed.push_back(crea->idNumber);
		}

		if(allowed.empty())
		{
			logGlobal->warn("Cannot pick a random creature of tier %d!", tier);
			return CreatureID::NONE;
		}

		return *RandomGeneratorUtil::nextItem(allowed, rand);
	}
	assert (r >= 0); //should always be, but it crashed
	return CreatureID(r);
}

void CCreatureHandler::addBonusForTier(int tier, const std::shared_ptr<Bonus> & b)
{
	assert(vstd::iswithin(tier, 1, 7));
	creaturesOfLevel[tier].addNewBonus(b);
}

void CCreatureHandler::addBonusForAllCreatures(const std::shared_ptr<Bonus> & b)
{
	const auto & exportedBonuses = allCreatures.getExportedBonusList();
	for(const auto & bonus : exportedBonuses)
	{
		if(bonus->type == b->type && bonus->subtype == b->subtype)
			return;
	}
	allCreatures.addNewBonus(b);
}

void CCreatureHandler::removeBonusesFromAllCreatures()
{
	allCreatures.removeBonuses(Selector::all);
}

void CCreatureHandler::buildBonusTreeForTiers()
{
	for(CCreature * c : objects)
	{
		if(vstd::isbetween(c->level, 0, ARRAY_COUNT(creaturesOfLevel)))
			c->attachTo(creaturesOfLevel[c->level]);
		else
			c->attachTo(creaturesOfLevel[0]);
	}
	for(CBonusSystemNode &b : creaturesOfLevel)
		b.attachTo(allCreatures);
}

void CCreatureHandler::afterLoadFinalization()
{

}

void CCreatureHandler::deserializationFix()
{
	buildBonusTreeForTiers();
}

VCMI_LIB_NAMESPACE_END
