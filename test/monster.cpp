#include "game.h"


bool Monster::StartEvilTurn()
{
	AP = Class->Speed;
	return true;
}

bool Monster::CanSeePlayer() const
{
	//return Game()->OurWorld->NavigationGrid().CanSee(Position(), Game()->CurrentPlayer->GetPosition(), true);
	return ParentMap()->NavGrid.Visible(Position());
	//return false;
}

bool Monster::CanAttackPlayer() const
{
	/*
	if (CanAttackDiagonally())
	return IsSurrounding(Position(), Game()->CurrentPlayer->GetPosition());
	else
	return IsNeighbor(Position(), Game()->CurrentPlayer->GetPosition());
	*/
	return false;
}

void Monster::AttackPlayer()
{

}

bool Monster::CanMoveTowardPlayer()
{
	const auto dir = DirToPlayer();
	if (IsDiagonal(dir))
		return mParentMap->CanCreatureMove(Position(), dir + 1) || mParentMap->CanCreatureMove(Position(), dir - 1);
	else
		return mParentMap->CanCreatureMove(Position(), dir);
}

void Monster::MoveTowardPlayer()
{
	const auto dir = DirToPlayer();
	if (IsCardinal(dir))
		MoveTo(Position() + ToVector(dir));
	else if (mParentMap->CanCreatureMove(Position(), dir+1))
		MoveTo(Position() + ToVector(dir+1));
	else if (mParentMap->CanCreatureMove(Position(), dir-1))
		MoveTo(Position() + ToVector(dir-1));
}

void Monster::Wander()
{
	/*
	auto wander_pos = GetPosition() + Game()->GetRandomNeighbor();
	if (Game()->CanEnter(wander_pos))
	SetPosition(wander_pos);
	*/
}

/*
if (Health() < ScaredHealth())
{
if (CanRunAwayFromPlayer())
RunAwayFromPlayer(); /// NOTE: Different than MoveAwayFromPlayer
else if (CanAttackPlayer())
AttackPlayer();
}
else if (TooFarFromPlayer() and CanAttackPlayer() and CanMoveTowardPlayer())
{
if (Game()->WithProbability(ChargeProbability()))
MoveTowardPlayer();
else
AttackPlayer();
}
else if (TooCloseToPlayer() and CanAttackPlayer() and CanMoveAwayFromPlayer())
{
if (Game()->WithProbability(RetreatProbability()))
MoveAwayFromPlayer();
else
AttackPlayer();
}
else if (CanAttackPlayer())
AttackPlayer();
else if (TooFarFromPlayer() and CanMoveTowardPlayer())
MoveTowardPlayer();
else if (TooCloseToPlayer() and CanMoveAwayFromPlayer())
MoveAwayFromPlayer();
else
StandStill();
*/

bool Monster::UpdateEvilTurn()
{
	return ParentMap()->ParentGame->SuspendableCall(mScriptObject, "UpdateEvilTurn");
}

Direction Monster::DirToPlayer() const
{
	const auto player_pos = ParentMap()->ParentGame->Player()->Position();
	const auto my_pos = Position();
	return ToDirection((radians_t)Angle(vec2{ player_pos - my_pos }));
}
