#include "game.h"


bool Monster::StartEvilTurn()
{
	mPathToPlayer.clear();
	AP = Class->Speed;
	return ParentMap()->ParentGame->Call<bool>(mScriptObject, "StartEvilTurn");
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
	if (!SpendAP()) return;
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
	if (!SpendAP()) return;
	/*
	auto wander_pos = GetPosition() + Game()->GetRandomNeighbor();
	if (Game()->CanEnter(wander_pos))
	SetPosition(wander_pos);
	*/
}

bool Monster::HasPathToPlayer()
{
	return !mPathToPlayer.empty();
}

bool Monster::CalculatePathToPlayer()
{
	mPathToPlayer.clear();

	const auto goal = mLastPlayerPosition;
	fmt::print("Searching for path to player from {} to {}... ", Position(), goal);
	auto new_path = mParentMap->NavGrid.AStarSearch<false>(
		Position(),
		goal, [&, goal](ivec2 from, ivec2 to) {
			return to == goal || mParentMap->CanCreatureMove(from, to);
		}, WallNavigationGrid::DefaultCostFunction, WallNavigationGrid::DefaultCostFunction
	);

	if (new_path.empty())
	{
		fmt::print("no path found.\n");
		return false;
	}

	mPathToPlayer = std::move(new_path);
	fmt::print("found:\n");
	for (auto& p : mPathToPlayer)
		fmt::print("  {}\n", p);
	return true;
}

bool Monster::MoveOnPath()
{
	fmt::print("Moving towards player... ");
	if (mPathToPlayer.empty())
	{
		fmt::print("done!\n");
		return false;
	}

	auto next_pos = mPathToPlayer.back();
	fmt::print("from {} to {}! ", Position(), next_pos);
	if (!mParentMap->CanCreatureMove(Position(), next_pos))
	{
		mPathToPlayer.clear();
		fmt::print("but can't move there, so restarting search :(\n");
		return false;
	}

	if (!SpendAP())
	{
		fmt::print("but no AP :(\n");
		return false;
	}

	mPathToPlayer.pop_back();
	MoveTo(next_pos);
	ParentMap()->ParentGame->CameraFollow(this, true);
	fmt::print("yay!\n");
	return true;
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
	if (AP)
	{
		ParentMap()->ParentGame->Call(mScriptObject, "UpdateEvilTurn");
		if (AP > 0)
			return false;
	}
	return true;
}

Direction Monster::DirToPlayer() const
{
	const auto player_pos = ParentMap()->ParentGame->Player()->Position();
	const auto my_pos = Position();
	return ToDirection((radians_t)Angle(vec2{ player_pos - my_pos }));
}

bool Monster::UpdateHeroTurn()
{
	if (CanSeePlayer())
		mLastPlayerPosition = mParentMap->ParentGame->Player()->Position();
	return false;
}
