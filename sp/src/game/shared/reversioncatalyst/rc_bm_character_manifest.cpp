//==============================================================================
//
// Purpose: This is Reversion Catalyst's recreation of Black Mesa: Source's character manifest system.
// 
// This is used to recreate its variety in character bodygroups and facial shapes.
// 
//==============================================================================

#include "cbase.h"
#include "rc_bm_character_manifest.h"
#include "igamesystem.h"
#include "filesystem.h"
#include "mapbase_matchers_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DEFAULT_CHARACTER_MANFIEST "scripts/character_manifest.txt"
#define DEFAULT_CHARACTER_MANFIEST_OVERRIDE "scripts/character_manifest_rc.txt"

extern char *AllocString( const char *pStr, int nMaxChars );

bool CCharacterManifestSystem::Init()
{
	return true;
}

void CCharacterManifestSystem::LevelInitPreEntity()
{
	g_CharacterManifestSystem.PurgeCharacterList();
	LoadCharacterManifestFile( DEFAULT_CHARACTER_MANFIEST, "GAME" );
	LoadCharacterManifestFile( DEFAULT_CHARACTER_MANFIEST_OVERRIDE, "MOD" );
}

void CCharacterManifestSystem::OnRestore()
{
}

void CCharacterManifestSystem::LevelInitPostEntity()
{
}

void CCharacterManifestSystem::LevelShutdownPreEntity()
{
}

void CCharacterManifestSystem::LevelShutdownPostEntity()
{
}

//-----------------------------------------------------------------------------

int CCharacterManifestSystem::SelectCharacterFromEntity( const char *pszEntName, const char *pszClassname )
{
	CUtlVector<int> specificCharacters;
	CUtlVector<int> classCharacters;
	for (int i = 0; i < m_CharacterList.Count(); i++)
	{
		if (FStrEq( m_CharacterList[i].pszName.String(), pszEntName ))
		{
			Msg( "Character '%s' (%i) matches classname '%s'\n", m_CharacterList[i].pszName.String(), i, pszEntName );
			specificCharacters.AddToTail( i );
		}
		else if (FStrEq( m_CharacterList[i].pszName.String(), pszClassname ))
		{
			Msg( "Character '%s' (%i) matches classname '%s'\n", m_CharacterList[i].pszName.String(), i, pszClassname );
			classCharacters.AddToTail( i );
		}
	}

	if (specificCharacters.Count() > 0)
	{
		return specificCharacters[RandomInt( 0, specificCharacters.Count() - 1 )];
	}
	else if (classCharacters.Count() > 0)
	{
		return classCharacters[RandomInt( 0, classCharacters.Count() - 1 )];
	}

	return -1;
}

Character_t *CCharacterManifestSystem::FindCharacterByName( const char *pszName )
{
	for (int i = 0; i < m_CharacterList.Count(); i++)
	{
		if (FStrEq( m_CharacterList[i].pszName.String(), pszName ))
		{
			return &m_CharacterList[i];
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------

inline void CCharacterManifestSystem::ParseIntList( const char *pszString, CUtlVector<int> &intList )
{
	CUtlStringList stringList;
	V_SplitString( pszString, ",", stringList );
	FOR_EACH_VEC( stringList, i )
	{
		intList.AddToTail( atoi( stringList[i] ) );
	}
}

void CCharacterManifestSystem::LoadCharacterManifestFile( const char *pszFile, const char *pathID )
{
	KeyValues *manifestKV = new KeyValues( "CharacterManifest" );
	if (manifestKV->LoadFromFile( filesystem, pszFile, pathID ))
	{
		for (KeyValues *pKey = manifestKV->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey())
		{
			// Parse this character
			Character_t *pCharacter = &m_CharacterList[m_CharacterList.AddToTail()];
			pCharacter->pszName = pKey->GetName();

			/*
			Character_t *pCharacter = FindCharacterByName( pKey->GetName() );
			if (!pCharacter)
			{
				pCharacter = &m_CharacterList[m_CharacterList.AddToTail()];
				pCharacter->pszName = pKey->GetName();
			}
			*/

#ifdef CLIENT_DLL
			KeyValues *pFlexDataKV = pKey->FindKey( "flex_data" );
			if (pFlexDataKV)
			{
				for (KeyValues *pFlex = pFlexDataKV->GetFirstSubKey(); pFlex; pFlex = pFlex->GetNextKey())
				{
					pCharacter->flexData.Insert( pFlex->GetName(), pFlex->GetFloat() );
				}
			}
#else
			pCharacter->pszModel = pKey->GetString( "model", "" );

			ParseIntList( pKey->GetString( "skin", "0" ), pCharacter->skins );

			KeyValues *pBodyDataKV = pKey->FindKey( "bodygroup_data" );
			if (pBodyDataKV)
			{
				for (KeyValues *pBody = pBodyDataKV->GetFirstSubKey(); pBody; pBody = pBody->GetNextKey())
				{
					int i = pCharacter->bodygroupData.Insert( pBody->GetName() );
					ParseIntList( pBody->GetString(), pCharacter->bodygroupData[i] );
				}
			}

			KeyValues *pBonemergeDataKV = pKey->FindKey( "bonemerge_data" );
			if (pBonemergeDataKV)
			{
				for (KeyValues *pBonemerge = pBonemergeDataKV->GetFirstSubKey(); pBonemerge; pBonemerge = pBonemerge->GetNextKey())
				{
					pCharacter->bonemergeData.AddToTail( AllocString( pBonemerge->GetName(), MAX_PATH ) );
				}
			}
#endif
		}
	}
	manifestKV->deleteThis();

	Msg( "Loaded character manifest with %i characters\n", m_CharacterList.Count() );
}

void CCharacterManifestSystem::PurgeCharacterList()
{
	m_CharacterList.Purge();
}

//-----------------------------------------------------------------------------

void CCharacterManifestSystem::PrintCharacter( const char *pszNameOrIndex )
{
	int iIndex = -1;
	const Character_t *pCharacter = NULL;
	if (AppearsToBeANumber( pszNameOrIndex ))
	{
		iIndex = atoi( pszNameOrIndex );
		if (iIndex >= 0 && iIndex < m_CharacterList.Count())
		{
			pCharacter = &m_CharacterList[iIndex];
		}
		else
		{
			Warning( "Character manifest print: Invalid index %i", iIndex );
			return;
		}
	}
	else
	{
		// Search by name
		for (int i = 0; i < m_CharacterList.Count(); i++)
		{
			if (FStrEq( m_CharacterList[i].pszName, pszNameOrIndex ))
			{
				pCharacter = &m_CharacterList[i];
				iIndex = i;
				break;
			}
		}
	}

	if (pCharacter)
	{
		// Print the character's information
		char szPrint[1024];

		Q_snprintf( szPrint, sizeof( szPrint ), "CHARACTER %i\n- Name: %s\n", iIndex, pCharacter->pszName.String() );

#ifdef CLIENT_DLL
		// Flex Data
		if (pCharacter->flexData.Count() > 0)
		{
			Q_strncat( szPrint, "- Flex Data\n", sizeof( szPrint ), sizeof( szPrint ) );

			for (unsigned int i = 0; i < pCharacter->flexData.Count(); i++)
			{
				Q_snprintf( szPrint, sizeof( szPrint ), "%s-- %s : %f\n", szPrint, pCharacter->flexData.GetElementName( i ), pCharacter->flexData[i] );
			}
		}
#else
		// Model
		if (pCharacter->pszModel.String()[0])
		{
			Q_snprintf( szPrint, sizeof( szPrint ), "%s- Model: %s\n", szPrint, pCharacter->pszModel.String() );
		}

		// Skins
		if (pCharacter->skins.Count() > 0)
		{
			Q_strncat( szPrint, "- Skins: (", sizeof( szPrint ), sizeof( szPrint ) );
			for (int i = 0; i < pCharacter->skins.Count(); i++)
			{
				Q_snprintf( szPrint, sizeof( szPrint ), "%s%i,", szPrint, pCharacter->skins[i] );
			}
			Q_strncat( szPrint, ")\n", sizeof( szPrint ), sizeof( szPrint ) );
		}

		// Bodygroups
		if (pCharacter->bodygroupData.Count() > 0)
		{
			Q_strncat( szPrint, "- Bodygroup Data\n", sizeof( szPrint ), sizeof( szPrint ) );

			for (unsigned int i = 0; i < pCharacter->bodygroupData.Count(); i++)
			{
				Q_snprintf( szPrint, sizeof( szPrint ), "%s-- %s (", szPrint, pCharacter->bodygroupData.GetElementName( i ) );
				for (int i2 = 0; i2 < pCharacter->bodygroupData[i].Count(); i2++)
				{
					Q_snprintf( szPrint, sizeof( szPrint ), "%s%i,", szPrint, pCharacter->bodygroupData[i][i2] );
				}
				Q_strncat( szPrint, ")\n", sizeof( szPrint ), sizeof( szPrint ) );
			}
		}

		// Bonemerges
		if (pCharacter->bonemergeData.Count() > 0)
		{
			Q_strncat( szPrint, "- Bonemerge Data\n", sizeof( szPrint ), sizeof( szPrint ) );

			for (int i = 0; i < pCharacter->bonemergeData.Count(); i++)
			{
				Q_snprintf( szPrint, sizeof( szPrint ), "%s-- %s\n", szPrint, pCharacter->bonemergeData[i] );
			}
		}
#endif

		Msg( "%s\n", szPrint );
	}
	else
	{
		Warning( "Character manifest print: Could not find character %s", pszNameOrIndex );
	}
}

//-----------------------------------------------------------------------------

CCharacterManifestSystem	g_CharacterManifestSystem;

BEGIN_DATADESC_NO_BASE( CCharacterManifestSystem )

	//DEFINE_UTLVECTOR( m_StoredManifestFiles, FIELD_STRING ),

END_DATADESC()

CON_COMMAND_SHARED( reload_character_manifest, "Reloads character manifest" )
{
	g_CharacterManifestSystem.PurgeCharacterList();
	g_CharacterManifestSystem.LoadCharacterManifestFile( DEFAULT_CHARACTER_MANFIEST, "GAME" );
	g_CharacterManifestSystem.LoadCharacterManifestFile( DEFAULT_CHARACTER_MANFIEST_OVERRIDE, "MOD" );
}

CON_COMMAND_SHARED( character_manifest_reload, "Reloads character manifest" )
{
	g_CharacterManifestSystem.PurgeCharacterList();
	g_CharacterManifestSystem.LoadCharacterManifestFile( DEFAULT_CHARACTER_MANFIEST, "GAME" );
	g_CharacterManifestSystem.LoadCharacterManifestFile( DEFAULT_CHARACTER_MANFIEST_OVERRIDE, "MOD" );
}

CON_COMMAND_SHARED( character_print, "Prints specified character" )
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: character_print <name or indx>\n");
		return;
	}

	g_CharacterManifestSystem.PrintCharacter( args.Arg( 1 ) );
}
