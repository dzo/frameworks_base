/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.internal.telephony;


/**
 * See also RIL_AppStatus in include/telephony/ril.h
 *
 * {@hide}
 */
public class IccCardApplication {
    public enum AppType{
        APPTYPE_UNKNOWN,
        APPTYPE_SIM,
        APPTYPE_USIM,
        APPTYPE_RUIM,
        APPTYPE_CSIM
    };

    public enum AppState{
        APPSTATE_UNKNOWN,
        APPSTATE_DETECTED,
        APPSTATE_PIN,
        APPSTATE_PUK,
        APPSTATE_SUBSCRIPTION_PERSO,
        APPSTATE_READY,
        APPSTATE_ILLEGAL;
         // Even if the app state is marked as Illegal by NAS, ICC IO
         // operations should be permitted. Hence treating the illegal
         // app state as card present. Network access requests will
         // anyway be rejected and ME will be in limited service.


        boolean isPinRequired() {
            return this == APPSTATE_PIN;
        }

        boolean isPukRequired() {
            return this == APPSTATE_PUK;
        }

        boolean isSubscriptionPersoEnabled() {
            return this == APPSTATE_SUBSCRIPTION_PERSO;
        }

        boolean isAppReady() {
            return ((this == APPSTATE_READY) || (this == APPSTATE_ILLEGAL));
        }

        boolean isAppNotReady() {
            return this == APPSTATE_UNKNOWN  ||
                   this == APPSTATE_DETECTED;
        }
    };

    public enum PersoSubState{
        PERSOSUBSTATE_UNKNOWN,
        PERSOSUBSTATE_IN_PROGRESS,
        PERSOSUBSTATE_READY,
        PERSOSUBSTATE_SIM_NETWORK,
        PERSOSUBSTATE_SIM_NETWORK_SUBSET,
        PERSOSUBSTATE_SIM_CORPORATE,
        PERSOSUBSTATE_SIM_SERVICE_PROVIDER,
        PERSOSUBSTATE_SIM_SIM,
        PERSOSUBSTATE_SIM_NETWORK_PUK,
        PERSOSUBSTATE_SIM_NETWORK_SUBSET_PUK,
        PERSOSUBSTATE_SIM_CORPORATE_PUK,
        PERSOSUBSTATE_SIM_SERVICE_PROVIDER_PUK,
        PERSOSUBSTATE_SIM_SIM_PUK,
        PERSOSUBSTATE_RUIM_NETWORK1,
        PERSOSUBSTATE_RUIM_NETWORK2,
        PERSOSUBSTATE_RUIM_HRPD,
        PERSOSUBSTATE_RUIM_CORPORATE,
        PERSOSUBSTATE_RUIM_SERVICE_PROVIDER,
        PERSOSUBSTATE_RUIM_RUIM,
        PERSOSUBSTATE_RUIM_NETWORK1_PUK,
        PERSOSUBSTATE_RUIM_NETWORK2_PUK,
        PERSOSUBSTATE_RUIM_HRPD_PUK,
        PERSOSUBSTATE_RUIM_CORPORATE_PUK,
        PERSOSUBSTATE_RUIM_SERVICE_PROVIDER_PUK,
        PERSOSUBSTATE_RUIM_RUIM_PUK;

        boolean isPersoSubStateUnknown() {
            return this == PERSOSUBSTATE_UNKNOWN;
        }
    };

    public AppType        app_type;
    public AppState       app_state;
    // applicable only if app_state == RIL_APPSTATE_SUBSCRIPTION_PERSO
    public PersoSubState  perso_substate;
    // null terminated string, e.g., from 0xA0, 0x00 -> 0x41, 0x30, 0x30, 0x30 */
    public String         aid;
    // null terminated string
    public String         app_label;
    // applicable to USIM and CSIM
    public int            pin1_replaced;
    public int            pin1;
    public int            pin2;

    AppType AppTypeFromRILInt(int type) {
        AppType newType;
        /* RIL_AppType ril.h */
        switch(type) {
            case 0: newType = AppType.APPTYPE_UNKNOWN; break;
            case 1: newType = AppType.APPTYPE_SIM;     break;
            case 2: newType = AppType.APPTYPE_USIM;    break;
            case 3: newType = AppType.APPTYPE_RUIM;    break;
            case 4: newType = AppType.APPTYPE_CSIM;    break;
            default:
                throw new RuntimeException(
                            "Unrecognized RIL_AppType: " +type);
        }
        return newType;
    }

    AppState AppStateFromRILInt(int state) {
        AppState newState;
        /* RIL_AppState ril.h */
        switch(state) {
            case 0: newState = AppState.APPSTATE_UNKNOWN;  break;
            case 1: newState = AppState.APPSTATE_DETECTED; break;
            case 2: newState = AppState.APPSTATE_PIN; break;
            case 3: newState = AppState.APPSTATE_PUK; break;
            case 4: newState = AppState.APPSTATE_SUBSCRIPTION_PERSO; break;
            case 5: newState = AppState.APPSTATE_READY; break;
            case 6: newState = AppState.APPSTATE_ILLEGAL; break;
            default:
                throw new RuntimeException(
                            "Unrecognized RIL_AppState: " +state);
        }
        return newState;
    }

    PersoSubState PersoSubstateFromRILInt(int substate) {
        PersoSubState newSubState;
        /* RIL_PeroSubstate ril.h */
        switch(substate) {
            case 0:  newSubState = PersoSubState.PERSOSUBSTATE_UNKNOWN;  break;
            case 1:  newSubState = PersoSubState.PERSOSUBSTATE_IN_PROGRESS; break;
            case 2:  newSubState = PersoSubState.PERSOSUBSTATE_READY; break;
            case 3:  newSubState = PersoSubState.PERSOSUBSTATE_SIM_NETWORK; break;
            case 4:  newSubState = PersoSubState.PERSOSUBSTATE_SIM_NETWORK_SUBSET; break;
            case 5:  newSubState = PersoSubState.PERSOSUBSTATE_SIM_CORPORATE; break;
            case 6:  newSubState = PersoSubState.PERSOSUBSTATE_SIM_SERVICE_PROVIDER; break;
            case 7:  newSubState = PersoSubState.PERSOSUBSTATE_SIM_SIM;  break;
            case 8:  newSubState = PersoSubState.PERSOSUBSTATE_SIM_NETWORK_PUK; break;
            case 9:  newSubState = PersoSubState.PERSOSUBSTATE_SIM_NETWORK_SUBSET_PUK; break;
            case 10: newSubState = PersoSubState.PERSOSUBSTATE_SIM_CORPORATE_PUK; break;
            case 11: newSubState = PersoSubState.PERSOSUBSTATE_SIM_SERVICE_PROVIDER_PUK; break;
            case 12: newSubState = PersoSubState.PERSOSUBSTATE_SIM_SIM_PUK; break;
            case 13: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_NETWORK1; break;
            case 14: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_NETWORK2; break;
            case 15: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_HRPD; break;
            case 16: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_CORPORATE; break;
            case 17: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_SERVICE_PROVIDER; break;
            case 18: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_RUIM; break;
            case 19: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_NETWORK1_PUK; break;
            case 20: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_NETWORK2_PUK; break;
            case 21: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_HRPD_PUK ; break;
            case 22: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_CORPORATE_PUK; break;
            case 23: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_SERVICE_PROVIDER_PUK; break;
            case 24: newSubState = PersoSubState.PERSOSUBSTATE_RUIM_RUIM_PUK; break;
            default:
                throw new RuntimeException(
                            "Unrecognized RIL_PersoSubstate: " +substate);
        }
        return newSubState;
    }

}
