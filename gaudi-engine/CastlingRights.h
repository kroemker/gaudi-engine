#pragma once
class CastlingRights
{
public:
	bool canQueenside;
	bool canKingside;

	CastlingRights(bool canKingside=true, bool canQueenside=true) {
		this->canKingside = canKingside;
		this->canQueenside = canQueenside;
	}

	void unsetAll() {
		canKingside = false;
		canQueenside = false;
	}
};

