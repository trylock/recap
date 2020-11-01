#include "recipe.hpp"

std::string recap::to_string(recipe::slot_t slot)
{
    switch (slot)
    {
        case recipe::SLOT_ALL:
            return "any";
            break;
        case recipe::SLOT_ARMOUR:
            return "armour";
            break;
        case recipe::SLOT_JEWELRY:
            return "jewelry";
            break;
        case recipe::SLOT_NONE:
            return "none";
            break;
        case recipe::SLOT_WEAPON1:
            return "weapon1";
            break;
        case recipe::SLOT_WEAPON2:
            return "weapon2";
            break;
        case recipe::SLOT_HELMET:
            return "helmet";
            break;
        case recipe::SLOT_BODY:
            return "body";
            break;
        case recipe::SLOT_GLOVES:
            return "gloves";
            break;
        case recipe::SLOT_BOOTS:
            return "boots";
            break;
        case recipe::SLOT_BELT:
            return "belt";
            break;
        case recipe::SLOT_RING1:
            return "ring1";
            break;
        case recipe::SLOT_RING2:
            return "ring2";
            break;
        case recipe::SLOT_AMULET:
            return "amulet";
            break;
    }
    return "<unknown>";
}

recap::recipe::slot_t recap::parse_slot(const std::string& slot)
{
    if (slot == "any") 
    {
        return recipe::SLOT_ALL;
    }
    else if (slot == "none") 
    {
        return recipe::SLOT_NONE;
    }
    else if (slot == "armour") 
    {
        return recipe::SLOT_ARMOUR;
    }
    else if (slot == "jewelry") 
    {
        return recipe::SLOT_JEWELRY;
    }
    else if (slot == "body") 
    {
        return recipe::SLOT_BODY;
    }
    else if (slot == "helmet") 
    {
        return recipe::SLOT_HELMET;
    }
    else if (slot == "weapon1") 
    {
        return recipe::SLOT_WEAPON1;
    }
    else if (slot == "weapon2") 
    {
        return recipe::SLOT_WEAPON2;
    }
    else if (slot == "gloves") 
    {
        return recipe::SLOT_GLOVES;
    }
    else if (slot == "belt") 
    {
        return recipe::SLOT_BELT;
    }
    else if (slot == "boots") 
    {
        return recipe::SLOT_BOOTS;
    }
    else if (slot == "ring1") 
    {
        return recipe::SLOT_RING1;
    }
    else if (slot == "ring2") 
    {
        return recipe::SLOT_RING2;
    }
    else if (slot == "amulet") 
    {
        return recipe::SLOT_AMULET;
    }
    return recipe::SLOT_NONE;
}