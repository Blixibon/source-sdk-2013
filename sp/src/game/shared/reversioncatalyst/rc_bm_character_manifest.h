//==============================================================================
//
// Purpose: This is Reversion Catalyst's recreation of Black Mesa: Source's character manifest system.
// 
// This is used to recreate its variety in character bodygroups and facial shapes.
// 
//==============================================================================

#include "cbase.h"

struct Character_t
{
	~Character_t()
	{
		pszName.Purge();
#ifdef GAME_DLL
		bonemergeData.PurgeAndDeleteElements();
		pszModel.Purge();
#endif
	}

	CUtlString					pszName;
#ifdef CLIENT_DLL
	CUtlDict<float>				flexData;
#else
	CUtlDict< CCopyableUtlVector<int> >	bodygroupData;
	CUtlStringList				bonemergeData; // TODO: Put on client?

	CUtlString					pszModel;
	CUtlVector<int>				skins;
#endif
};

//-----------------------------------------------------------------------------
// Purpose: System used to load map-specific files, etc.
//-----------------------------------------------------------------------------
class CCharacterManifestSystem : public CAutoGameSystem
{
public:
	DECLARE_DATADESC();

	CCharacterManifestSystem() : CAutoGameSystem( "CCharacterManifestSystem" )
	{
	}

	virtual bool Init();
	virtual void LevelInitPreEntity();
	virtual void OnRestore();
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPreEntity();
	virtual void LevelShutdownPostEntity();

	//-----------------------------------------------------------------------------

	int SelectCharacterFromEntity( const char *pszEntName, const char *pszClassname );

	Character_t *FindCharacterByName( const char *pszName );

	inline const Character_t &GetCharacter( int i )
	{
		Assert( i < m_CharacterList.Count() );
		return m_CharacterList[i];
	}

	inline int GetCharacterCount() const { return m_CharacterList.Count(); }

	//-----------------------------------------------------------------------------

	void ParseIntList( const char *pszString, CUtlVector<int> &intList );

	void LoadCharacterManifestFile( const char *pszFile, const char *pathID = NULL );
	void PurgeCharacterList();

	//-----------------------------------------------------------------------------

	void PrintCharacter( const char *pszNameOrIndex );

	//-----------------------------------------------------------------------------

private:

	CUtlVector<Character_t>		m_CharacterList;
};

extern CCharacterManifestSystem	g_CharacterManifestSystem;
