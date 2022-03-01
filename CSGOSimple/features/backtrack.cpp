/*#include <algorithm>
#include <array>

*/

struct BacktrackingRecord
{
    int Tick;
    Vector Position;
    BacktrackingRecord(int iTick, Vector vPos)
    {
        Tick = iTick;
        Position = vPos;
    }
};

namespace LegitBacktracking
{
    extern std::vector<BacktrackingRecord> PlayerRecords[64];

    void CM(void);
    void PT(void);
    void FN(void);
}

namespace LegitBacktracking
{
    std::vector<BacktrackingRecord> PlayerRecords[64];

    void CM(void)
    {

        float flBestFov = FLT_MAX;
        int   iBestTarget = -1;

        for (auto Enemy : Globals::ValidEnemy)
        {
            int iIndex = Enemy->GetIndex();

            if (PlayerRecords[iIndex].size() > static_cast<size_t>(20))
            {
                PlayerRecords[iIndex].pop_back();
            }

            if (!PlayerRecords[iIndex].empty())
            {
                if (PlayerRecords[iIndex].front().Tick == Globals::UserCmd->tick_count)
                    continue;
            }

            matrix3x4_t Matrix[128];
            if (!Enemy->SetupBones(Matrix, 128, 256, 0))
                continue;

            Vector vPoint;
            if (!Helpers::GetHitBox(Enemy, 12, vPoint, Matrix))
                continue;

            BacktrackingRecord CurRecord = BacktrackingRecord(Globals::UserCmd->tick_count, vPoint);
            PlayerRecords[iIndex].insert(PlayerRecords[iIndex].begin(), CurRecord);

            Vector CalcAngled = CalcAngel(Globals::LocalPlayer->GetEyePosition(), vPoint);
            CalcAngled -= Globals::LocalPlayer->GetPunch() * 2.f;


            float fov = Helpers::GetFov(Globals::UserCmd->viewangles, CalcAngled);
            if (fov < flBestFov)
            {
                flBestFov = fov;
                iBestTarget = Enemy->GetIndex();
            }
        }

        if (![censored]Shit::IsAttacking || ![censored]Shit::CanAttacking)
            return;

        if (iBestTarget != -1)
        {
            float Temp = FLT_MAX;
            for (auto tick : PlayerRecords[iBestTarget])
            {
                Vector CalcAngled = CalcAngel(Globals::LocalPlayer->GetEyePosition(), tick.Position);
                CalcAngled -= Globals::LocalPlayer->GetPunch() * 2.f;


                float fov = Helpers::GetFov(Globals::UserCmd->viewangles, CalcAngled);
                if (fov < Temp)
                {
                    Temp = fov;
                    Globals::UserCmd->tick_count = tick.Tick;
                }
            }
        }
    }

    void PT(void)
    {
        for (auto Enemy : Globals::ValidEnemy)
        {
            int iIndex = Enemy->GetIndex();
            for (auto x : PlayerRecords[iIndex])
            {
                Vector HeadPos;
                if (!Helpers::WorldToScreen(x.Position, HeadPos))
                    return;

                Drawing::Drawline(HeadPos.x, HeadPos.y, HeadPos.x + 2, HeadPos.y + 2, Color::Green());
            }
        }
    }
}