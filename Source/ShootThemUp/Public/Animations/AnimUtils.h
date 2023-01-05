#pragma once

class AnimUtils
{
public:
    template <typename T>
    static T* FindNotifyByClass(UAnimSequenceBase* Animation)
    {
        if (!Animation) return nullptr;

        const auto NotifyEvents = Animation->Notifies;
        for (auto NotifyEvent : NotifyEvents)
        {
            auto AnimNotify = Cast<T>(NotifyEvent.Notify);
            auto ChangeWeapon = Cast<T>(NotifyEvent.Notify);
            
            if (AnimNotify)
            {
                return AnimNotify;
            }
            else if(ChangeWeapon)
            {
                return ChangeWeapon;
            }
        }
        return nullptr;
    }
};
