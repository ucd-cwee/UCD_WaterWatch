#pragma once
#include "Precompiled.h"
#if 0
namespace cweeAsset {
	using namespace cwee_units;
	namespace epanet {
		namespace details {
			static ::epanet::StatusType		GetCurrentStatus(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k) {
				if (k) {
					AUTO ptr = k->GetValue<_STATUS_>(); 
					if (ptr) {
						return (::epanet::StatusType)(u64)ptr->GetCurrentValue((u64)pr->times->Htime);
					}
				}
				return ::epanet::CLOSED;
			};
			static gallon_per_minute_t		GetCurrentFlow(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k) {
				if (k) {
					AUTO ptr = k->GetValue<_FLOW_>();
					if (ptr) {
						return ptr->GetCurrentValue((u64)pr->times->Htime);
					}
				}
				return 0;
			};
			static gallon_per_minute_t		GetCurrentDemand(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeNode> k) {
				if (k) {
					AUTO ptr = k->GetValue<_DEMAND_>();
					if (ptr) {
						return ptr->GetCurrentValue((u64)pr->times->Htime);
					}
				}
				return 0;
			};
			static foot_t					GetCurrentHead(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeNode> k) {
				if (k) {
					AUTO ptr = k->GetValue<_HEAD_>();
					if (ptr) {
						return ptr->GetCurrentValue((u64)pr->times->Htime);
					}
				}
				return 0;
			};
			static foot_t					GetCurrentHeadloss(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k) {
				if (k) {
					return GetCurrentHead(pr, k->StartingAsset) - GetCurrentHead(pr, k->EndingAsset);
				}
				return 0;
			};
			static scalar_t					GetCurrentSetting(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k) {
				if (k) {
					AUTO ptr = k->GetValue<_SETTING_>();
					if (ptr) {
						return ptr->GetCurrentValue((u64)pr->times->Htime);
					}
				}
				return 0;
			};
			static foot_t					GetCurrentTankLevel(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeReservoir> k) {
				if (k) {
					AUTO ptr = k->GetValue<_HEAD_>();
					if (ptr) {
						return ptr->GetCurrentValue((u64)pr->times->Htime) - (foot_t)k->Coordinates.GetExclusive()->z;
					}
				}
				return 0;
			};
			static cweeUnion<kilowatt_t, scalar_t>  calc_pump_energy(cweeSharedPtr<Project> pr, cweeSharedPtr<cweePump> k) {
				cweeUnion<kilowatt_t, scalar_t> out;

				AUTO net = pr->network;
				AUTO hyd = pr->hydraul;
				AUTO time = pr->times;

				int    i,       // efficiency curve index
					j;       // pump index

				foot_t dh;					// head across pump (ft)
				cubic_foot_per_second_t q;	// flow through pump (cfs)
				scalar_t e;					// pump efficiency

				double q4eff;   // flow at nominal pump speed of 1.0
				scalar_t speed;   // current speed setting


				if (GetCurrentStatus(pr, k.CastReference<cweeLink>()) <= ::epanet::CLOSED) {
					return out;
				}

				// Determine flow and head difference
				q = units::math::fabs(GetCurrentFlow(pr, k.CastReference<cweeLink>()));
				dh = units::math::fabs(GetCurrentHeadloss(pr, k.CastReference<cweeLink>()));

				// For pumps, find effic. at current flow
				
				if (k->Type == asset_t::PUMP) {			
					speed = GetCurrentSetting(pr, k);
					e = 100.0 - ((100.0 - k->EfficiencyAtFlow(q)()) * pow(1.0 / speed(), 0.1)); // Sarbu and Borza pump speed adjustment
					e = units::math::fmin(e, scalar_t(100.0));
					e = units::math::fmax(e, scalar_t(1.0));
					e /= 100.0;
				}
				else {
					e = 1.0;
				}

				// Compute energy
				out.get<0>() = cweeEng::CentrifugalPumpEnergyDemand_kW(q, dh, e*100);
				out.get<1>() = e;

				return out;
			};
			static void  getallpumpsenergy(cweeSharedPtr<Project> pr){
				for (auto& h : pr->network->pumps.HashList()) {
					AUTO ptr = pr->network->pumps.Find(h);
					AUTO energy_efficiency = calc_pump_energy(pr, ptr);
					ptr->GetValue<_ENERGY_>()->AddUniqueValue((u64)pr->times->Htime, energy_efficiency.get<0>());
					// ptr->GetValue<_EFFICIENCY_>()->AddUniqueValue((u64)pr->times->Htime, energy_efficiency.get<1>());
				}
			};

			namespace HydraulicSim {
                static auto& LinkFlow(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k) {
                    AUTO p = pr->hydraul->LinkFlow.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k;
                    return p->get<1>();
                };
                // P = sq.ft / sec?
                static auto& P(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k) {
                    AUTO p = pr->hydraul->P.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k;
                    return p->get<1>();
                };
                // Y = cubic foot / sec?
                static auto& Y(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k) {
                    AUTO p = pr->hydraul->Y.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k;
                    return p->get<1>();
                };
                static auto& LinkStatus(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k) {
                    AUTO p = pr->hydraul->LinkStatus.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k.CastReference<cweeAsset>();
                    return p->get<1>();
                };
                static auto& LinkSetting(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k) {
                    AUTO p = pr->hydraul->LinkSetting.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k;
                    return p->get<1>();
                };
                static auto& NodeHead(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeNode> k) {
                    AUTO p = pr->hydraul->NodeHead.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k;
                    return p->get<1>();
                };
                static auto& NodeDemand(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeNode> k) {
                    AUTO p = pr->hydraul->NodeDemand.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k;
                    return p->get<1>();
                };
                static auto& DemandFlow(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeNode> k) {
                    AUTO p = pr->hydraul->DemandFlow.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k;
                    return p->get<1>();
                };
                static auto& EmitterFlow(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeNode> k) {
                    AUTO p = pr->hydraul->EmitterFlow.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k;
                    return p->get<1>();
                };
                static auto& Xflow(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeAsset> k) {
                    AUTO p = pr->hydraul->Xflow.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k;
                    return p->get<1>();
                };
                static auto& OldStatus(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeAsset> k) {
                    AUTO p = pr->hydraul->OldStatus.FindOrMake(k->Hash());
                    if (!p->get<0>()) p->get<0>() = k;
                    return p->get<1>();
                };

                static constexpr scalar_t Scalar(double t) { return (scalar_t)(double)t; };
                template<typename T, typename = std::enable_if<units::traits::is_unit_t<T>::value>> static constexpr scalar_t Scalar(T t) { return (scalar_t)(double)t; };
                template<typename T> static constexpr scalar_t Scalar(cweeInterlocked<T> t) { return (scalar_t)(double)t.Read(); };

				static int		allocmatrix(cweeSharedPtr<Project> pr) {
					AUTO net = pr->network;
					AUTO hyd = pr->hydraul;

					int errcode = 0;

					/* P */
					for (auto& h : net->links) {
						AUTO ptr = hyd->P.FindOrMake(h->Hash());
						ptr->get<0>() = h;
					}
					/* Y */
					for (auto& h : net->links) {
						AUTO ptr = hyd->Y.FindOrMake(h->Hash());
						ptr->get<0>() = h;
					}
					/* X-Flow */
					for (auto& h : net->assets) {
						AUTO ptr = hyd->Xflow.FindOrMake(h->Hash());
						ptr->get<0>() = h;
					}
					/* OldStatus */
					for (auto& h : net->assets) {
						AUTO ptr = hyd->OldStatus.FindOrMake(h->Hash());
						ptr->get<0>() = h;
					}
					return errcode;
				};
				static void		initlinkflow(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> link) {
					AUTO hyd = pr->hydraul;
					AUTO n = pr->network;

					if ((int)link->GetCurrentValue<_STATUS_>((u64)pr->times->GetProjectStartTime()) == (int)::epanet::CLOSED) {
                        LinkFlow(pr, link) = ::epanet::QZERO;
					}
					else if (link->Type == asset_t::PUMP) {
                        AUTO pump = n->pumps.Find(link->Hash());
                        LinkFlow(pr, link) = link->Kc_Roughness.Read() * pump->FlowAtHead(0); //  hyd->LinkFlow[i] = link->Kc_Roughness * n->Pump[findpump(n, i)].Q0;
					}
					else {
                        LinkFlow(pr, link) = (cweeMath::PI * ::epanet::SQR(link->Diameter.Read()) / 4.0) * (feet_per_second_t)(1.0);
					}
				};
				static int		unlinked(cweeSharedPtr<Project> pr) {
					AUTO net = pr->network;
					int i, count = 0;
					for (auto& junc : net->junctions) {
						AUTO e = pr->network->Adjlist.Find(junc.Hash());
						if (!e || !e->get<1>()) {
							count++;
							cweeStr::printf("Error 234: %s %s", errmsg(234).c_str(), junc.Name.Read().c_str());
						}
						if (count >= 10) break;
					}
					if (count > 0) return 233;
					return 0;
				};
				static int		openhyd(cweeSharedPtr<Project> pr){
					int  i;
					int  errcode = 0;
					
					// Check for too few nodes & no fixed grade nodes
					if (pr->network->nodes.Num() < 2) errcode = 223;
					else if (pr->network->reservoirs.Num() == 0) errcode = 224;

					pr->times->Htime = pr->times->GetProjectStartTime();

					// Allocate memory for sparse matrix structures (see SMATRIX.C)
					smatrix_t::createsparse(pr->network, pr->hydraul->smatrix);

					// Allocate memory for hydraulic variables
					allocmatrix(pr);

					// Check for unconnected nodes
					unlinked(pr);

					// Initialize link flows
					for (auto& link : pr->network->links) {
						initlinkflow(pr, link);
					}
				
					return errcode;
				};

				static cweeUnion<second_t, cweeSharedPtr<cweeReservoir>>  tanktimestep(cweeSharedPtr<Project> pr) {
					cweeUnion<second_t, cweeSharedPtr<cweeReservoir>> out;

					AUTO net = pr->network;
					AUTO hyd = pr->hydraul;
					AUTO time = pr->times;

					foot_t h;
					cubic_foot_per_second_t q;
					int     i, n, tankIdx = 0;
					million_gallon_t  v;

					// Examine each tank
					for (auto& hash : net->reservoirs.HashList()) {
						AUTO ptr = net->reservoirs.Find(hash);
						if (!ptr->TerminalStorage.Read()) { // Skip reservoirs
							// Get current tank grade (h) & inflow (q)
							h = GetCurrentHead(pr, ptr.CastReference<cweeNode>());
							q = GetCurrentDemand(pr, ptr.CastReference<cweeNode>());
							if (units::math::fabs(q) <= (gallon_per_minute_t)::epanet::QZERO) continue;

							// Find volume to fill/drain tank						
							if (q > 0.0_gpm && h < ptr->MaxHead()) v = ptr->MaxVolume() - ptr->VolumeAt(GetCurrentTankLevel(pr, ptr));
							else if (q < 0.0_gpm && h > ptr->MinHead()) v = ptr->MinVolume() - ptr->VolumeAt(GetCurrentTankLevel(pr, ptr));
							else continue;

							// Find time to fill/drain tank
							second_t t_thisTank = v / q;
							if (t_thisTank > 0_s && (t_thisTank < out.get<0>() || out.get<0>() == 0_s)) {
								out.get<0>() = t_thisTank;
								out.get<1>() = ptr;
							}
						}
					}

					return out;
				};
				static second_t controltimestep(cweeSharedPtr<Project> pr) {
					second_t tStep = 0_s;

					AUTO net = pr->network;
					AUTO hyd = pr->hydraul;
					AUTO time = pr->times;

					int    i, j, k, n;
					double h, q, v;
					long   t, t1, t2;

					// Examine each control
					for (auto& hash : net->Controllers.HashList()) {
						AUTO control = net->Controllers.Find(hash);
						for (auto& x : *control->StatusControllers.GetExclusive()) {
							if (x) {
								AUTO bool_time = x->EstimateNextTriggerTime(time->Htime);
								if (bool_time.get<0>() && bool_time.get<1>() > 0 && ((second_t)(u64)(bool_time.get<1>() - time->Htime) < tStep || tStep == 0_s)) {
									tStep = (second_t)(u64)(bool_time.get<1>() - time->Htime);
								}
							}
						}
					}

					return tStep;
				};

				static void		tanklevels(cweeSharedPtr<Project> pr, second_t tstep) {
					AUTO net = pr->network;
					AUTO hyd = pr->hydraul;
					AUTO time = pr->times;

					int    i, n;
					double dv;

					for (auto& hash : net->reservoirs.HashList()) {
						AUTO res = net->reservoirs.Find(hash);
						if (!res->TerminalStorage.Read()) { // tanks only...
							// Update the tank's water elevation
							AUTO ptr = res->GetValue<_HEAD_>();
							AUTO ptr2 = res->GetValue<_DEMAND_>();
							if (ptr && ptr2) {
								million_gallon_t added_gallons = ptr2->GetCurrentValue((u64)time->Htime) * tstep;
								foot_t lvl = res->LvlAtVolume(added_gallons + res->VolumeAt(ptr->GetCurrentValue((u64)time->Htime) - (foot_t)res->Coordinates.GetExclusive()->z));
								ptr->AddUniqueValue((u64)(time->Htime + (u64)(second_t)tstep), lvl + (foot_t)res->Coordinates.GetExclusive()->z);
							}
						}
					}					
				};
				static second_t timestep(cweeSharedPtr<Project> pr) {
					AUTO net = pr->network;
					AUTO hyd = pr->hydraul;
					AUTO time = pr->times;

					long n, t;
					second_t tstep;

					// default time step is hydraulic time step
					tstep = time->Hstep;

					// Revise time step based on time until next demand period
					{
						scalar_t fractionalRemainder = units::math::fmod((scalar_t)(u64)(second_t)(u64)(time->Htime - time->GetProjectStartTime()), (scalar_t)(u64)(second_t)time->Pstep);
						second_t tstep_to_next_pat_step = (scalar_t(1) - fractionalRemainder) * time->Pstep;
						if (tstep_to_next_pat_step <= second_t(0)) tstep_to_next_pat_step = time->Pstep;
						tstep = units::math::fmin(tstep_to_next_pat_step, tstep);
					}

					// Revise time step based on smallest time to fill or drain a tank
					{
						AUTO tstep_to_next = tanktimestep(pr);
						if (tstep_to_next.get<1>() && tstep_to_next.get<0>() > 0_s) { // if a reservoir was selected with a valid time... 
							tstep = units::math::fmin(tstep_to_next.get<0>(), tstep);
						}
					}

					// Revise time step based on smallest time to activate a control
					{
						AUTO tstep_to_next = controltimestep(pr); 
						if (tstep_to_next > 0_s) { 
							tstep = units::math::fmin(tstep_to_next, tstep);
						}
					}

					// Update tank levels based on the demands of the tanks
					{
						tanklevels(pr, tstep);
					}

					return tstep;
				};
				static  second_t	nexthyd(cweeSharedPtr<Project> pr)	{
					
					AUTO hyd = pr->hydraul;
					AUTO time = pr->times;

					second_t  hydstep;         // Actual time step
					int   errcode = 0;     // Error code

					// Compute current power and efficiency of all pumps
					getallpumpsenergy(pr);

					// Save current results to hydraulics file and force end of simulation if Haltflag is active
					if (hyd->Haltflag) time->Htime = time->GetProjectStartTime() + (u64)time->Dur;

					// Compute next time step & update tank levels
					hydstep = 0;
					if (time->Htime < (time->GetProjectStartTime() + (u64)time->Dur)) hydstep = timestep(pr);

					// More time remains - update current time
					if (time->Htime < (time->GetProjectStartTime() + (u64)time->Dur))
					{
						time->Htime += (u64)hydstep;
					}

					// No more time remains - force completion of analysis
					else
					{
						time->Htime += 1;
						//if (pr->quality.OpenQflag) time->Qtime++;
						hydstep = -1;
					}

					//--> RG
					//time->Hstep = hydstep;
					//<-- RG

					return hydstep;
				};
				static void     resetpumpflow(cweeSharedPtr<Project> pr, cweeSharedPtr<cweePump> pump)	{
					AUTO net = pr->network;

					if (pump->HeadCurveMode.Read() == ::epanet::CONST_HP) {
						AUTO f = pr->hydraul->LinkFlow.FindOrMake(pump->Hash());
						if (!f->get<0>()) f->get<0>() = pump.CastReference<cweeLink>();
						f->get<1>() = 0.0_gpm;
						pump->GetValue<_FLOW_>()->AddUniqueValue((u64)pr->times->Htime, 0_gpm);
					}
				};
				static void     setlinksetting(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> link, scalar_t value, scalar_t* s, scalar_t* k) {
					AUTO net = pr->network;
					auto& t = link->Type;

					AUTO pump = link.CastReference<cweePump>();
					AUTO valve = link.CastReference<cweeValve>();
					scalar_t zero = 0;

					// For a pump, status is OPEN if speed > 0, CLOSED otherwise
					if (pump)
					{
						*k = value;
						if (value > zero && *s <= Scalar(::epanet::CLOSED))
						{
							// Check if a re-opened pump needs its flow reset
							resetpumpflow(pr, pump);
							*s = Scalar(::epanet::OPEN);
						}
						if (value == zero && *s > Scalar(::epanet::CLOSED)) *s = Scalar(::epanet::CLOSED);
					}

					// For FCV, activate it
					else if (valve && valve->valveType == valveType_t::FCV)
					{
						*k = value;
						*s = Scalar(::epanet::ACTIVE);
					}

					// Open closed control valve with fixed status (setting = MISSING)
					else
					{
						if (*k == Scalar(::epanet::MISSING) && *s <= Scalar(::epanet::CLOSED)) *s = Scalar(::epanet::OPEN);
						*k = value;
					}
				};
				static void  demands(cweeSharedPtr<Project> pr)	{
					AUTO net = pr->network;
					AUTO hyd = pr->hydraul;
					AUTO time = pr->times;
					AUTO currentTime = time->Htime;

					int  i, j, n;
					long k, p;
					double djunc;
					gallon_per_minute_t sum;

					// Update demand at each node according to its assigned pattern
					hyd->Dsystem = 0.0_gpm;          // System-wide demand
					for (auto& junc : net->junctions) { // for each junction...
						sum = junc.DemandAt(currentTime) * hyd->Dmult; // Dmult is a system scalar for customer demands
						hyd->Dsystem += sum;

						// Set the demand
						{
							AUTO f = hyd->NodeDemand.FindOrMake(junc.Hash());
							if (!f->get<0>()) f->get<0>() = net->junctions.Find(junc.Hash()).CastReference<cweeNode>();
							f->get<1>() = sum;
							junc.GetValue<_DEMAND_>()->AddUniqueValue((u64)currentTime, sum);
						}

						// Initialize pressure dependent demand
						{
							AUTO f = hyd->DemandFlow.FindOrMake(junc.Hash());
							if (!f->get<0>()) f->get<0>() = net->junctions.Find(junc.Hash()).CastReference<cweeNode>();
							f->get<1>() = sum;
						}
					}
					net->systemwide->GetValue<_DEMAND_>()->AddUniqueValue((u64)currentTime, hyd->Dsystem);

					// Update head at fixed grade nodes with time patterns
					for (auto& res : net->reservoirs) {
						if (!res.TerminalStorage.Read()) {

						}
						else {
							if (res.HeadPattern) {
								foot_t head = res.HeadPattern->GetCurrentValue((u64)currentTime);
								{
									AUTO f = hyd->NodeHead.FindOrMake(res.Hash());
									if (!f->get<0>()) f->get<0>() = net->reservoirs.Find(res.Hash()).CastReference<cweeNode>();
									f->get<1>() = head;
									res.GetValue<_HEAD_>()->AddUniqueValue((u64)currentTime, head);
								}
							}

						}
					}

					// Update status of pumps with utilization (pre-determined) pattern
					for (auto& pump : net->pumps) {
						if (pump.PreDeterminedStatusPattern) {
							scalar_t settingOrStatus = pump.PreDeterminedStatusPattern->GetCurrentValue((u64)currentTime);
							AUTO status = hyd->LinkStatus.FindOrMake(pump.Hash());
							if (!status->get<0>()) status->get<0>() = net->pumps.Find(pump.Hash()).CastReference<cweeAsset>();
							
							AUTO setting = hyd->LinkSetting.FindOrMake(pump.Hash());
							if (!setting->get<0>()) setting->get<0>() = net->pumps.Find(pump.Hash()).CastReference<cweeAsset>();

							setlinksetting(pr, net->pumps.Find(pump.Hash()).CastReference<cweeLink>(), settingOrStatus , &status->get<1>(), &setting->get<1>());
							pump.GetValue<_STATUS_>()->AddUniqueValue((u64)currentTime, status->get<1>());
							pump.GetValue<_SETTING_>()->AddUniqueValue((u64)currentTime, setting->get<1>());							
						}
					}
				};
				static void  controls(cweeSharedPtr<Project> pr)	{
					AUTO net = pr->network;
					AUTO hyd = pr->hydraul;
					AUTO time = pr->times;

					int i, k, n, reset, setsum;
					foot_t h;
					double vplus;
					double v1, v2;
					scalar_t k1, k2;
					scalar_t  s1, s2;

					for (auto& contoller : net->Controllers) {
						auto link = contoller.Parent;
						if (link) {
							AUTO nextStatus = contoller.TryGetNextStatus((u64)time->Htime, (u64)time->Htime - (u64)time->Hstep);
							AUTO nextSetting = contoller.TryGetNextSetting((u64)time->Htime, (u64)time->Htime - (u64)time->Hstep);

							if (nextStatus || nextSetting) {
								// reset!
                                auto& currentStatus = LinkStatus(pr, link);
                                auto& currentSetting = LinkSetting(pr, link);

								if (nextStatus) {
									if (currentStatus <= Scalar(::epanet::CLOSED)) s1 = Scalar(::epanet::CLOSED);
									else s1 = Scalar(::epanet::OPEN);
									s2 = *nextStatus;

									if (link->Type == asset_t::PUMP && s1 == Scalar(::epanet::CLOSED) && s2 == Scalar(::epanet::OPEN)) resetpumpflow(pr, net->pumps.Find(link->Hash()));

									currentStatus = s2;
									AUTO ptr = link->GetValue<_STATUS_>();
									if (ptr) {
										ptr->AddUniqueValue((u64)pr->times->Htime, currentStatus);
									}
								}
								if (nextSetting) {
									k1 = currentSetting;
									k2 = currentSetting;
									if (link->Type == asset_t::PUMP || link->Type == asset_t::VALVE) {
										currentSetting = *nextSetting;

										AUTO ptr = link->GetValue<_SETTING_>();
										if (ptr) {
											ptr->AddUniqueValue((u64)pr->times->Htime, currentSetting);
										}
									}
								}
							}
						}
					}
				};

                static scalar_t frictionFactor(cubic_foot_per_second_t q, scalar_t e, cubic_foot_per_second_t s, scalar_t* dfdq)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   q = |pipe flow|
                    **            e = pipe roughness  / diameter
                    **            s = viscosity * pipe diameter
                    **   Output:  dfdq = derivative of friction factor w.r.t. flow
                    **   Returns: pipe's friction factor
                    **   Purpose: computes Darcy-Weisbach friction factor and its derivative as a function of Reynolds Number (Re).
                    **--------------------------------------------------------------
                    */
                {
                    scalar_t f;                // friction factor
                    scalar_t x1,
                        x2, 
                        x3, 
                        x4, 
                        y1, 
                        y2, 
                        y3, 
                        fa, 
                        fb, 
                        r;
                    scalar_t w = q / s;        // Re*Pi/4
                    
                    //   For Re >= 4000 use Swamee & Jain approximation  of the Colebrook-White Formula
                    if (w >= Scalar(::epanet::A1))
                    {
                        y1 = Scalar(::epanet::A8) / std::pow((double)w, 0.9);
                        y2 = e / Scalar(3.7) + y1;
                        y3 = Scalar(::epanet::A9) * units::math::log(y2);
                        f = Scalar(1.0) / (y3 * y3);
                        *dfdq = Scalar(1.8) * f * y1 * Scalar(::epanet::A9) / y2 / y3 / Scalar(q);
                    }
                    //   Use interpolating polynomials developed by E. Dunlop for transition flow from 2000 < Re < 4000.
                    else
                    {
                        y2 = e / 3.7 + Scalar(::epanet::AB);
                        y3 = Scalar(::epanet::A9) * std::log((double)y2);
                        fa = Scalar(1.0) / (y3 * y3);
                        fb = (Scalar(2.0) + Scalar(::epanet::AC) / (y2 * y3)) * fa;
                        r = w / Scalar(::epanet::A2);
                        x1 = Scalar(7.0) * fa - fb;
                        x2 = Scalar(0.128) - Scalar(17.0) * fa + Scalar(2.5) * fb;
                        x3 = Scalar(-0.128) + Scalar(13.0) * fa - (fb + fb);
                        x4 = Scalar(0.032) - Scalar(3.0) * fa + 0.5 * Scalar(fb);
                        f = x1 + r * (x2 + r * (x3 + r * x4));
                        *dfdq = (x2 + r * (Scalar(2.0) * x3 + r * Scalar(3.0) * x4)) / Scalar(s) / Scalar(::epanet::A2);
                    }
                    return f;
                };

                static void DWpipecoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> link)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   k = link index
                    **   Output:  none
                    **   Purpose: computes pipe head loss coeffs. for Darcy-Weisbach
                    **            formula.
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    auto q = units::math::fabs((cubic_foot_per_second_t)LinkFlow(pr, link));
                    auto r = link->R_FlowResistance.Read();                         // Resistance coeff.
                    auto ml = link->Km_MinorLoss.Read();                       // Minor loss coeff.
                    auto e = link->Kc_Roughness.Read() / link->Diameter.Read();           // Relative roughness
                    cubic_foot_per_second_t s = hyd->Viscos * link->Diameter.Read();        // Viscosity / diameter
                    foot_t hloss;
                    ft_per_cfs_t hgrad;
                    scalar_t f, dfdq, r1;

                    scalar_t sixteenPI = cweeMath::PI * 16.0;

                    // Compute head loss and its derivative
                    // ... use Hagen-Poiseuille formula for laminar flow (Re <= 2000)
                    if (q <= (s * Scalar(::epanet::A2)))
                    {
                        r = Scalar(sixteenPI * s * r);
                        hloss = (double)Scalar(LinkFlow(pr, link) * Scalar(r + ml * Scalar(q)));
                        hgrad = (double)r + (double)Scalar(Scalar(2.0) * ml * q);
                    }
                    // ... otherwise use Darcy-Weisbach formula with friction factor
                    else
                    {
                        dfdq = 0.0;
                        f = frictionFactor(q, (double)e, s, &dfdq);
                        r1 = f * r + ml;
                        hloss = (double)Scalar(r1 * q * LinkFlow(pr, link));
                        hgrad = (double)Scalar(2.0 * r1 * q) + (double)Scalar(dfdq * r * q * q);
                    }

                    // Compute P and Y coefficients
                    P(pr, link) = (Scalar(1.0) / hgrad);
                    Y(pr, link) = (hloss / hgrad);
                };
                static void  resistcoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k)  {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;
                    k->Calculate_R_FlowResistance((::epanet::HeadLossType)hyd->Formflag, (scalar_t)hyd->Hexp);
                };
                static void  pipecoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   k = link index
                    **   Output:  none
                    **   Purpose:  computes P & Y coefficients for pipe k.
                    **
                    **    P = inverse head loss gradient = 1/hgrad
                    **    Y = flow correction term = hloss / hgrad
                    **--------------------------------------------------------------
                    */
                {
                    using namespace cwee_units;

                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    foot_t  hloss;     // Head loss
                    ft_per_cfs_t hgrad;     // Head loss gradient        

                    // For closed pipe use headloss formula: hloss = CBIG*q
                    if (LinkStatus(pr, k) <= Scalar(::epanet::CLOSED))
                    {
                        P(pr, k) = 1.0 / ::epanet::CBIG;
                        Y(pr, k) = LinkFlow(pr, k);
                        return;
                    }

                    // Use custom function for Darcy-Weisbach formula
                    if (hyd->Formflag == ::epanet::DW)
                    {
                        DWpipecoeff(pr, k);
                        return;
                    }

                    AUTO q = units::math::fabs(LinkFlow(pr, k)); // Abs. value of flow
                    AUTO ml = k->Km_MinorLoss.Read(); // Minor loss coeff.
                    AUTO r = k->R_FlowResistance.Read(); // Resistance coeff.

                    // Friction head loss gradient
                    hgrad = (double)(Scalar(hyd->Hexp) * r * Scalar(::pow((double)q, hyd->Hexp - 1.0)));

                    AUTO hloss_cwee = k->Calculate_FlowHeadloss((::epanet::HeadLossType)hyd->Formflag, LinkFlow(pr, k), (double)hyd->Viscos);

                    std::cout << cweeStr::printf("\tWaW PIPE %s\n\t\t", k->Name->c_str());
                    std::cout << "Diameter: " << k->Diameter.Read() << ", ";
                    std::cout << "Kc_Roughness: " << k->Kc_Roughness.Read() << ", ";
                    std::cout << "Km_MinorLoss: " << k->Km_MinorLoss.Read() << ", ";
                    std::cout << "Length: " << k->Length.Read() << ";";
                    std::cout << "\n\t\t";
                    std::cout << "Flow: " << q << ", ";
                    std::cout << "Resistance: " << r << ", ";
                    std::cout << "Resistance_cwee_approx?: " << k->Calculate_R_FlowResistance((::epanet::HeadLossType)hyd->Formflag, hyd->Hexp) << ", ";
                    std::cout << "hgrad: " << hgrad << ", ";
                    std::cout << "hgrad_cwee_approx?: " << hloss_cwee / (q / Scalar(hyd->Hexp)) << ", ";
                    std::cout << "RQtol: " << hyd->RQtol << ", ";
                    std::cout << "Hexp: " << hyd->Hexp << ";";
                    std::cout << std::endl;                   

                    // Friction head loss:
                    // ... use linear function for very small gradient
                    if (Scalar(hgrad) < Scalar(hyd->RQtol))
                    {
                        hgrad = hyd->RQtol;
                        hloss = hgrad * q;
                    }
                    // ... otherwise use original formula
                    else hloss = hgrad * q / Scalar(hyd->Hexp);

                    // Contribution of minor head loss
                    if (ml > 0.0)
                    {
                        hloss += (decltype(hloss))(double)Scalar(ml * q * q);
                        hgrad += (decltype(hgrad))(double)Scalar(2.0) * Scalar(ml) * Scalar(q);
                    }

                    // Adjust head loss sign for flow direction
                    hloss *= Scalar(::epanet::SGN((double)LinkFlow(pr, k)));

                    // P and Y coeffs.
                    P(pr, k) = 1.0 / hgrad;
                    Y(pr, k) = hloss / hgrad;
                };
                static void  pumpcoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   k = link index
                    **   Output:  none
                    **   Purpose: computes P & Y coeffs. for pump in link k
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    scalar_t n;                // Flow exponent coeff.
                    foot_t  hloss;     // Head loss
                    ft_per_cfs_t hgrad;     // Head loss gradient       
                   
                    cweeSharedPtr<cweePump> pump = net->pumps.Find(k->Hash());

                    // Use high resistance pipe if pump closed or cannot deliver head
                    AUTO setting = LinkSetting(pr, k); // Pump speed setting
                    if (!pump || LinkStatus(pr, k) <= Scalar(::epanet::CLOSED) || setting == 0.0)
                    {
                        P(pr, k) = 1.0 / ::epanet::CBIG;
                        Y(pr, k) = LinkFlow(pr, k);
                    }
                    else {
                        // Obtain reference to pump object
                        AUTO q = units::math::fabs((cubic_foot_per_second_t)LinkFlow(pr, k)); // Abs. value of flow

                        switch (pump->HeadCurveMode.Read()) {
                        case ::epanet::NOCURVE: { // If no pump curve treat pump as an open valve
                            P(pr, k) = 1.0 / ::epanet::CSMALL;
                            Y(pr, k) = LinkFlow(pr, k);                            
                        } break;
                        case ::epanet::CUSTOM: { // Get pump curve coefficients for custom pump curve (Other pump types have pre-determined coeffs.)
                            // Find intercept (h0) & slope (r) of pump curve line segment which contains speed-adjusted flow.
                            if (pump->HeadCurve) {
                                AUTO h0 = pump->HeadCurve->GetCurrentValue(q / setting);  // Shutoff head                            
                                ft_per_cfs_t r = pump->HeadCurve->GetCurrentFirstDerivative(q / setting); // Flow resistance coeff.

                                // Determine head loss coefficients (negative sign converts from pump curve's head gain to head loss)
                                //pump->H0 = -h0;
                                //pump->R = -r;
                                //pump->N = 1.0;

                                // Compute head loss and its gradient (with speed adjustment)
                                hgrad = -r * setting;
                                hloss = -h0 * ::epanet::SQR(setting) + hgrad * LinkFlow(pr, k);

                                if (hgrad > (ft_per_cfs_t)::epanet::CSMALL || hgrad < (ft_per_cfs_t)(-::epanet::CSMALL)) {
                                    P(pr, k) = 1.0 / hgrad;
                                    Y(pr, k) = hloss / hgrad;
                                }
                                else {
                                    P(pr, k) = 1.0 / ::epanet::CSMALL;
                                    Y(pr, k) = LinkFlow(pr, k);
                                }
                            }
                            else {
                                P(pr, k) = 1.0 / ::epanet::CSMALL;
                                Y(pr, k) = LinkFlow(pr, k);
                            }                           
                        } break;
                        default: 
                        case ::epanet::CONST_HP:
                        case ::epanet::POWER_FUNC: {
                            if (pump->HeadCurve) {
                                AUTO h0 = pump->HeadCurve->GetCurrentValue(q / setting);  // Shutoff head                            
                                ft_per_cfs_t r = pump->HeadCurve->GetCurrentFirstDerivative(q / setting); // Flow resistance coeff.

                                // Adjust head loss coefficients for pump speed
                                h0 = ::epanet::SQR(setting) * -h0;
                                r = -r * Scalar(std::pow((double)setting, 1.0));


                                // Constant HP pump
                                if (pump->HeadCurveMode == ::epanet::CONST_HP)
                                {
                                    // ... compute pump curve's gradient
                                    hgrad = ((double)-r) / ((double)q) / ((double)q);

                                    // ... use linear curve if gradient too large or too small
                                    if (Scalar(hgrad) > Scalar(::epanet::CBIG)) {
                                        hgrad = ::epanet::CBIG;
                                        hloss = -hgrad * LinkFlow(pr, k);
                                    }
                                    else if (Scalar(hgrad) < Scalar(hyd->RQtol)) {

                                        hgrad = hyd->RQtol;
                                        hloss = -hgrad * LinkFlow(pr, k);
                                    }
                                    else {  // ... otherwise compute head loss from pump curve                                
                                        hloss = r * LinkFlow(pr, k);
                                    }
                                }

                                // Compute head loss and its gradient
                                // ... pump curve is nonlinear
                                //else if (n != 1.0)
                                //{
                                //    // ... compute pump curve's gradient
                                //    hgrad = n * r * Scalar(std::pow(q, n - 1.0));
                                //    // ... use linear pump curve if gradient too small
                                //    if (hgrad < hyd->RQtol)
                                //    {
                                //        hgrad = hyd->RQtol;
                                //        hloss = h0 + hgrad * LinkFlow(pr, k);
                                //    }
                                //    // ... otherwise compute head loss from pump curve
                                //    else hloss = h0 + hgrad * LinkFlow(pr, k) / n;
                                //}
                                // ... pump curve is linear
                                //else
                                {
                                    hgrad = r;
                                    hloss = h0 + hgrad * LinkFlow(pr, k);
                                }

                                P(pr, k) = 1.0 / hgrad;
                                Y(pr, k) = hloss / hgrad;
                            }
                            else {
                                P(pr, k) = 1.0 / ::epanet::CSMALL;
                                Y(pr, k) = LinkFlow(pr, k);
                            }
                        } break;
                        }
                    }

                    std::cout << cweeStr::printf("\tWaW PUMP %s\n\t\t", k->Name->c_str());
                    std::cout << "Diameter: " << k->Diameter.Read() << ", ";
                    std::cout << "Kc_Roughness: " << k->Kc_Roughness.Read() << ", ";
                    std::cout << "Km_MinorLoss: " << k->Km_MinorLoss.Read() << ", ";
                    std::cout << "Length: " << k->Length.Read() << ";";
                    std::cout << "\n\t\t";
                    std::cout << "Flow: " << LinkFlow(pr, k) << ", ";
                    std::cout << "hgrad: " << 1.0 / P(pr, k) << ", ";
                    std::cout << "P: " << P(pr, k) << ", ";
                    std::cout << "Y: " << Y(pr, k) << ", ";
                    std::cout << "hloss: " << Y(pr, k) / P(pr, k) << ";";
                    std::cout << std::endl;
                };
                
                static void valvecoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   k    = link index
                    **   Output:  none
                    **   Purpose: computes solution matrix coeffs. for a completely
                    **            open, closed, or throttled control valve.
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    // Valve is closed. Use a very small matrix coeff.
                    if (LinkStatus(pr, k) <= Scalar(::epanet::CLOSED))
                    {
                        P(pr, k) = 1.0 / ::epanet::CBIG;
                        Y(pr, k) = LinkFlow(pr, k);
                        return;
                    }

                    // Account for any minor headloss through the valve
                    if (Scalar(k->Km_MinorLoss) > Scalar(0.0))
                    {
                        AUTO flow = LinkFlow(pr, k);

                        AUTO q = units::math::fabs(flow);
                        ft_per_cfs_t hgrad = (double)(q * Scalar(2.0) * k->Km_MinorLoss.Read());

                        // Guard against too small a head loss gradient
                        if (Scalar(hgrad) < Scalar(hyd->RQtol))
                        {
                            hgrad = hyd->RQtol;

                            // P and Y coeffs.
                            P(pr, k) = 1.0 / hgrad;
                            Y(pr, k) = flow;                                                     
                        }
                        else {
                            // P and Y coeffs.
                            P(pr, k) = 1.0 / hgrad;
                            Y(pr, k) = flow / 2.0;
                        }
                    }                    
                    else // If no minor loss coeff. specified use a low resistance linear head loss relation
                    {
                        P(pr, k) = 1.0 / ::epanet::CSMALL;
                        Y(pr, k) = LinkFlow(pr, k);
                    }
                };
                static void  gpvcoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   k = link index
                    **   Output:  none
                    **   Purpose: computes P & Y coeffs. for general purpose valve
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    int    i;
                    double h0 = 0,        // Intercept of head loss curve segment
                        r = 0;         // Slope of head loss curve segment

                    cweeSharedPtr<cweeValve> valve = net->valves.Find(k->Hash());
                    if (!valve) { return; }

                    // Treat as a pipe if valve closed
                    if (LinkStatus(pr, k) == Scalar(::epanet::CLOSED)) valvecoeff(pr, k);                    
                    else // Otherwise utilize segment of head loss curve bracketing current flow
                    { 
                        // Adjusted flow rate
                        AUTO q = units::math::fabs(LinkFlow(pr, k)); // Abs. value of flow
                        q = units::math::max(q, (decltype(q))::epanet::TINY);
                        if (valve->HeadlossCurve) {
                            // Intercept and slope of curve segment containing q
                            AUTO h0 = valve->HeadlossCurve->GetCurrentValue(q);  // Shutoff head                            
                            ft_per_cfs_t r = valve->HeadlossCurve->GetCurrentFirstDerivative(q); // Flow resistance coeff.

                            r = units::math::max(r, (decltype(r))::epanet::TINY);

                            // Resulting P and Y coeffs.
                            P(pr, k) = 1.0 / r;
                            Y(pr, k) = ((h0 / r) + q) * ::epanet::SGN(Scalar(LinkFlow(pr, k)));
                        }
                        else {
                            P(pr, k) = 1.0 / ::epanet::CBIG;
                            Y(pr, k) = LinkFlow(pr, k);
                            return;
                        }
                    }
                };
                static void  pbvcoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   k = link index
                    **   Output:  none
                    **   Purpose: computes P & Y coeffs. for pressure breaker valve
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    // If valve fixed OPEN or CLOSED then treat as a pipe
                    if (LinkSetting(pr, k) == ::epanet::MISSING || LinkSetting(pr, k) == 0.0)
                    {
                        valvecoeff(pr, k);
                    }

                    // If valve is active
                    else
                    {
                        // Treat as a pipe if minor loss > valve setting
                        if (Scalar(k->Km_MinorLoss.Read() * ::epanet::SQR(LinkFlow(pr, k))) > LinkSetting(pr, k))
                        {
                            valvecoeff(pr, k);
                        }                        
                        else // Otherwise force headloss across valve to be equal to setting
                        {
                            P(pr, k) = ::epanet::CBIG;
                            Y(pr, k) = (double)(LinkSetting(pr, k) * Scalar(::epanet::CBIG));
                        }
                    }
                };
                static void  tcvcoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   k = link index
                    **   Output:  none
                    **   Purpose: computes P & Y coeffs. for throttle control valve
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    // Save original loss coeff. for open valve
                    AUTO km = k->Km_MinorLoss.Read();

                    // If valve not fixed OPEN or CLOSED, compute its loss coeff.
                    if (LinkSetting(pr, k) != Scalar(::epanet::MISSING))
                    {
                        k->Km_MinorLoss = Scalar(0.02517) * LinkSetting(pr, k) / Scalar(::epanet::SQR(k->Diameter.Read()) * ::epanet::SQR(k->Diameter.Read()));
                    }

                    // Then apply usual valve formula
                    valvecoeff(pr, k);

                    // Restore original loss coeff.
                    k->Km_MinorLoss = km;
                };
                static void  prvcoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k, cweeSharedPtr<cweeNode> n1, cweeSharedPtr<cweeNode> n2)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   k    = link index
                    **            n1   = upstream node of valve
                    **            n2   = downstream node of valve
                    **   Output:  none
                    **   Purpose: computes solution matrix coeffs. for pressure
                    **            reducing valves
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;                    
                    AUTO time = pr->times;

                    int   i, j;                        // Rows of solution matrix
                    double hset;                       // Valve head setting

                    i = hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, n1)];                  // Matrix rows of nodes
                    j = hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, n2)];
                    hset = n2->Coordinates.GetExclusive()->z + LinkSetting(pr, k);        // Valve setting

                    if (LinkStatus(pr, k) == Scalar(::epanet::ACTIVE))
                    {
                        // Set coeffs. to force head at downstream node equal to valve setting & force flow to equal to flow excess at downstream node.
                        P(pr, k) = 0.0;
                        Y(pr, k) = LinkFlow(pr, k) + Xflow(pr, n2.CastReference<cweeAsset>());   // Force flow balance
                        hyd->smatrix->F[j] += (cubic_foot_per_second_t)(hset * ::epanet::CBIG);                        // Force head = hset
                        hyd->smatrix->Aii[j] += (cfs_p_ft_t)::epanet::CBIG;                               // at downstream node
                        if (Xflow(pr, n2.CastReference<cweeAsset>()) < 0.0_cfs)
                        {
                            hyd->smatrix->F[i] += Xflow(pr, n2.CastReference<cweeAsset>());
                        }
                        return;
                    }

                    // For OPEN, CLOSED, or XPRESSURE valve, compute matrix coeffs. using the valvecoeff() function.
                    valvecoeff(pr, k);
                    hyd->smatrix->Aij[hyd->smatrix->Ndx[smatrix_t::LinkToIndex(hyd->smatrix, k)]] -= P(pr, k);
                    hyd->smatrix->Aii[i] += P(pr, k);
                    hyd->smatrix->Aii[j] += P(pr, k);
                    hyd->smatrix->F[i] += (Y(pr, k) - LinkFlow(pr, k));
                    hyd->smatrix->F[j] -= (Y(pr, k) - LinkFlow(pr, k));
                };
                static void  psvcoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k, cweeSharedPtr<cweeNode> n1, cweeSharedPtr<cweeNode> n2)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   k    = link index
                    **            n1   = upstream node of valve
                    **            n2   = downstream node of valve
                    **   Output:  none
                    **   Purpose: computes solution matrix coeffs. for pressure
                    **            sustaining valve
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    int   i, j;                        // Rows of solution matrix
                    double hset;                       // Valve head setting

                    i = hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, n1)];                  // Matrix rows of nodes
                    j = hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, n2)];
                    hset = n1->Coordinates.GetExclusive()->z + LinkSetting(pr, k);        // Valve setting

                    if (LinkStatus(pr, k) == Scalar(::epanet::ACTIVE))
                    {
                        // Set coeffs. to force head at downstream node equal to valve setting & force flow to equal to flow excess at downstream node.
                        P(pr, k) = 0.0;
                        Y(pr, k) = LinkFlow(pr, k) + Xflow(pr, n1.CastReference<cweeAsset>());   // Force flow balance
                        hyd->smatrix->F[j] += (cubic_foot_per_second_t)(hset * ::epanet::CBIG);                        // Force head = hset
                        hyd->smatrix->Aii[j] += (cfs_p_ft_t)::epanet::CBIG;                               // at downstream node
                        if (Xflow(pr, n1.CastReference<cweeAsset>()) < 0.0_cfs)
                        {
                            hyd->smatrix->F[i] += Xflow(pr, n1.CastReference<cweeAsset>());
                        }
                        return;
                    }

                    // For OPEN, CLOSED, or XPRESSURE valve, compute matrix coeffs. using the valvecoeff() function.
                    valvecoeff(pr, k);
                    hyd->smatrix->Aij[hyd->smatrix->Ndx[smatrix_t::LinkToIndex(hyd->smatrix, k)]] -= P(pr, k);
                    hyd->smatrix->Aii[i] += P(pr, k);
                    hyd->smatrix->Aii[j] += P(pr, k);
                    hyd->smatrix->F[i] += (Y(pr, k) - LinkFlow(pr, k));
                    hyd->smatrix->F[j] -= (Y(pr, k) - LinkFlow(pr, k));
                };
                static void  fcvcoeff(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k, cweeSharedPtr<cweeNode> n1, cweeSharedPtr<cweeNode> n2)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   k    = link index
                    **            n1   = upstream node of valve
                    **            n2   = downstream node of valve
                    **   Output:  none
                    **   Purpose: computes solution matrix coeffs. for flow control
                    **            valve
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    int   i, j;                   // Rows in solution matrix

                    cubic_foot_per_second_t q = (double)LinkSetting(pr, k); // Valve flow setting
                    i = hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, n1)];                  // Matrix rows of nodes
                    j = hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, n2)];

                    // If valve active, break network at valve and treat
                    // flow setting as external demand at upstream node
                    // and external supply at downstream node.

                    if (LinkStatus(pr, k) == Scalar(::epanet::ACTIVE))
                    {
                        Xflow(pr, n1.CastReference<cweeAsset>()) -= q;
                        Xflow(pr, n2.CastReference<cweeAsset>()) += q;
                        Y(pr, k) = LinkFlow(pr, k) - q;
                        hyd->smatrix->F[i] -= q;
                        hyd->smatrix->F[j] += q;
                        P(pr, k) = 1.0 / ::epanet::CBIG;
                        hyd->smatrix->Aij[hyd->smatrix->Ndx[smatrix_t::LinkToIndex(hyd->smatrix, k)]] -= P(pr, k);
                        hyd->smatrix->Aii[i] += P(pr, k);
                        hyd->smatrix->Aii[j] += P(pr, k);
                    }                    
                    else { // Otherwise treat valve as an open pipe
                        valvecoeff(pr, k);
                        hyd->smatrix->Aij[hyd->smatrix->Ndx[smatrix_t::LinkToIndex(hyd->smatrix, k)]] -= P(pr, k);
                        hyd->smatrix->Aii[i] += P(pr, k);
                        hyd->smatrix->Aii[j] += P(pr, k);
                        hyd->smatrix->F[i] += (Y(pr, k) - LinkFlow(pr, k));
                        hyd->smatrix->F[j] -= (Y(pr, k) - LinkFlow(pr, k));
                    }
                };

                static void headlosscoeffs(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   none
                    **   Output:  none
                    **   Purpose: computes coefficients P (1 / head loss gradient)
                    **            and Y (head loss / gradient) for all links.
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    for (auto& k : net->links) {
                        switch (k->Type) {
                        case asset_t::PIPE: {
                            pipecoeff(pr, k);
                            break;
                        }
                        case asset_t::VALVE: {
                            AUTO valve = net->valves.Find(k->Hash());
                            switch (valve->valveType) {
                            case valveType_t::PBV:
                                pbvcoeff(pr, k);
                                break;
                            case valveType_t::TCV:
                                tcvcoeff(pr, k);
                                break;
                            case valveType_t::GPV:
                                gpvcoeff(pr, k);
                                break;
                            case valveType_t::FCV:
                            case valveType_t::PRV:
                            case valveType_t::PSV:
                                if (LinkSetting(pr, k) == ::epanet::MISSING) valvecoeff(pr, k);
                                else P(pr, k) = 0.0; 
                                break;
                            }  
                            break;
                        }
                        case asset_t::PUMP: {
                            pumpcoeff(pr, k);
                            break;
                        }
                        }
                    }
                };
                static void  linkcoeffs(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   none
                    **   Output:  none
                    **   Purpose: computes coefficients contributed by links to the
                    **            linearized system of hydraulic equations.
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    // Examine each link of network
                    for (auto& link : net->links) {
                        if (P(pr, link) == (cfs_p_ft_t)0.0) continue;
                        if (P(pr, link) > (cfs_p_ft_t)::epanet::CBIG) {
                            std::cout << "P was very large" << std::endl;
                        }
                        if (P(pr, link) < (cfs_p_ft_t)(-::epanet::CBIG)) {
                            std::cout << "P was very negative" << std::endl;
                        }


                        AUTO n1 = link->StartingAsset; // Start node of link
                        AUTO n2 = link->EndingAsset; // End node of link   

                        // Update nodal flow excess (Xflow)
                        // (Flow out of node is (-), flow into node is (+))
                        Xflow(pr, n1.CastReference<cweeAsset>()) -= LinkFlow(pr, link);
                        Xflow(pr, n2.CastReference<cweeAsset>()) += LinkFlow(pr, link);

                        // Add to off-diagonal coeff. of linear system matrix
                        hyd->smatrix->Aij[hyd->smatrix->Ndx[ smatrix_t::LinkToIndex(hyd->smatrix, link) ]] -= P(pr, link);

                        // Update linear system coeffs. associated with start node n1
                        
                        if (n1->Type == asset_t::JUNCTION) 
                        {                           
                            int n1_index = smatrix_t::NodeToIndex(hyd->smatrix, n1);
                            hyd->smatrix->Aii[hyd->smatrix->Row[n1_index]] += P(pr, link); // Diagonal coeff.
                            hyd->smatrix->F[hyd->smatrix->Row[n1_index]] += Y(pr, link); // RHS coeff.
                        }
                        else if (n1->Type == asset_t::RESERVOIR) {
                            hyd->smatrix->F[hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, n2)]] += P(pr, link) * NodeHead(pr, n1);
                        }

                        // Update linear system coeffs. associated with end node n2
                        if (n1->Type == asset_t::JUNCTION)
                        {
                            int n2_index = smatrix_t::NodeToIndex(hyd->smatrix, n2);
                            hyd->smatrix->Aii[hyd->smatrix->Row[n2_index]] += P(pr, link); // Diagonal coeff.
                            hyd->smatrix->F[hyd->smatrix->Row[n2_index]] -= Y(pr, link); // RHS coeff.
                        }
                        else if (n1->Type == asset_t::RESERVOIR) {
                            hyd->smatrix->F[hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, n1)]] += P(pr, link) * NodeHead(pr, n2);
                        }
                    }
                };
                static void emitterheadloss(cweeSharedPtr<Project> pr, cweeJunction& node, foot_t& hloss, ft_per_cfs_t& hgrad)
                    /*
                    **-------------------------------------------------------------
                    **   Input:   i = node index
                    **   Output:  hloss = head loss across node's emitter
                    **            hgrad = head loss gradient
                    **   Purpose: computes an emitters's head loss and gradient.
                    **-------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    double  ke;
                    cubic_foot_per_second_t  q;

                    // Set adjusted emitter coeff.
                    ke = std::fmax(::epanet::CSMALL, node.Ke);

                    // Compute gradient of head loss through emitter
                    q = EmitterFlow(pr, *net->nodes.Find(node.Hash()));
                    hgrad = hyd->Qexp * ke * pow((double)units::math::fabs(q), hyd->Qexp - 1.0);
                    
                    if (hgrad < (ft_per_cfs_t)hyd->RQtol) // Use linear head loss function for small gradient
                    {
                        hgrad = hyd->RQtol;
                        hloss = hgrad * q;
                    }                   
                    else hloss = (hgrad * q) / Scalar(hyd->Qexp);  // Otherwise use normal emitter head loss function
                };
                static void  emittercoeffs(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   none
                    **   Output:  none
                    **   Purpose: computes coeffs. of the linearized hydraulic eqns.
                    **            contributed by emitters.
                    **
                    **   Note: Emitters consist of a fictitious pipe connected to
                    **         a fictitious reservoir whose elevation equals that
                    **         of the junction. The headloss through this pipe is
                    **         Ke*(Flow)^hyd->Qexp, where Ke = emitter headloss coeff.
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    int    i, row;
                    foot_t hloss;
                    ft_per_cfs_t hgrad;

                    for (auto& junc : net->junctions) {
                        // Skip junctions without emitters
                        if (junc.Ke == 0.0) continue;

                        // Find emitter head loss and gradient
                        emitterheadloss(pr, junc, hloss, hgrad);

                        // Row of solution matrix
                        row = hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, net->FindNode(junc.Name))];

                        // Addition to matrix diagonal & r.h.s
                        hyd->smatrix->Aii[row] += 1.0 / hgrad;
                        hyd->smatrix->F[row] += (hloss + (foot_t)junc.Coordinates.GetExclusive()->z) / hgrad;

                        // Update to node flow excess
                        Xflow(pr, *net->assets.Find(junc.Hash())) -= EmitterFlow(pr, *net->nodes.Find(junc.Hash()));
                    }
                };                              
                static void demandheadloss(cweeSharedPtr<Project> pr, cweeJunction& node, foot_t dp, double n, foot_t& hloss, ft_per_cfs_t& hgrad)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   i  = junction index
                    **            dp = pressure range for demand function (ft)
                    **            n  = exponent in head v. demand function
                    **   Output:  hloss = pressure dependent demand head loss (ft)
                    **            hgrad = gradient of head loss (ft/cfs)
                    **  Purpose:  computes head loss and its gradient for delivering
                    **            a pressure dependent demand flow.
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    AUTO node_index = smatrix_t::NodeToIndex(hyd->smatrix, net->FindNode(node.Name));
                    AUTO node_ptr = *net->nodes.Find(node.Hash());
                    
                    cubic_foot_per_second_t d = DemandFlow(pr, node_ptr);
                    cubic_foot_per_second_t dfull = NodeDemand(pr, node_ptr);
                    scalar_t r = d / dfull;

                    // Use lower barrier function for negative demand
                    if (r <= scalar_t(0))
                    {
                        hgrad = ::epanet::CBIG;
                        hloss = ::epanet::CBIG * (double)d;
                    }

                    // Use power head loss function for demand less than full
                    else if (r < 1.0)
                    {
                        hgrad = n * dp * pow(r, n - 1.0) / dfull;
                        // ... use linear function for very small gradient
                        if (hgrad < (ft_per_cfs_t)hyd->RQtol)
                        {
                            hgrad = (ft_per_cfs_t)hyd->RQtol;
                            hloss = hgrad * d;
                        }
                        else hloss = (hgrad * d) / Scalar(n);
                    }

                    // Use upper barrier function for demand above full value
                    else
                    {
                        hgrad = ::epanet::CBIG;
                        hloss = dp + (foot_t)(double)(Scalar(::epanet::CBIG) * (d - dfull));
                    }
                };
                static void  demandcoeffs(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   none
                    **   Output:  none
                    **   Purpose: computes coeffs. of the linearized hydraulic eqns.
                    **            contributed by pressure dependent demands.
                    **
                    **   Note: Pressure dependent demands are modelled like emitters
                    **         with Hloss = Preq * (D / Dfull)^(1/Pexp)
                    **         where D (actual demand) is zero for negative pressure
                    **         and is Dfull above pressure Preq.
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    int row;
                    foot_t  dp;         // pressure range over which demand can vary (ft)
                    double   n;          // exponent in head loss v. demand function
                    foot_t   hloss;      // head loss in supplying demand (ft)
                    ft_per_cfs_t   hgrad;      // gradient of demand head loss (ft/cfs)

                // Get demand function parameters
                    if (hyd->DemandModel == ::epanet::DDA) return;
                    dp = (foot_t)(double)(head_t)(hyd->Preq - hyd->Pmin);
                    n = 1.0 / hyd->Pexp;

                    // Examine each junction node
                    for (auto& junc : net->junctions) {
                        // Skip junctions with non-positive demands
                        if (NodeDemand(pr, net->FindNode(junc.Name)) <= 0.0_cfs) continue;

                        // Find head loss for demand outflow at node's elevation
                        demandheadloss(pr, junc, dp, n, hloss, hgrad);

                        // Update row of solution matrix A & its r.h.s. F
                        if (hgrad > (ft_per_cfs_t)0.0)
                        {
                            row = hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, net->FindNode(junc.Name))];
                            hyd->smatrix->Aii[row] += 1.0 / hgrad;
                            hyd->smatrix->F[row] += (hloss + (foot_t)junc.Coordinates.GetExclusive()->z + (foot_t)(double)(head_t)hyd->Pmin) / hgrad;
                        }
                    }
                };
                static void  nodecoeffs(cweeSharedPtr<Project> pr)
                    /*
                    **----------------------------------------------------------------
                    **  Input:   none
                    **  Output:  none
                    **  Purpose: completes calculation of nodal flow balance array
                    **           (Xflow) & r.h.s. (F) of linearized hydraulic eqns.
                    **----------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    // For junction nodes, subtract demand flow from net flow excess & add flow excess to RHS array F
                    for (auto& junc : net->nodes) {
                        if (junc->Type == asset_t::JUNCTION) {
                            Xflow(pr, junc.CastReference<cweeAsset>()) -= DemandFlow(pr, junc);
                            hyd->smatrix->F[hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, junc)]] += Xflow(pr, junc.CastReference<cweeAsset>());
                        }
                    }
                };
                static void  valvecoeffs(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   none
                    **   Output:  none
                    **   Purpose: computes coeffs. of the linearized hydraulic eqns.
                    **            contributed by PRVs, PSVs & FCVs whose status is
                    **            not fixed to OPEN/CLOSED
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    int i, k, n1, n2;

                    // Examine each valve
                    for (auto& link : net->links) {
                        if (link->Type == asset_t::VALVE) {
                            AUTO valve = net->valves.Find(link->Hash());
                            if (valve) {
                                // Coeffs. for fixed status valves have already been computed
                                if (LinkSetting(pr, link) == ::epanet::MISSING) continue;

                                // Start & end nodes of valve's link
                                AUTO n1 = link->StartingAsset;
                                AUTO n2 = link->EndingAsset;

                                // Call valve-specific function
                                switch (valve->valveType)
                                {
                                case valveType_t::PRV:
                                    prvcoeff(pr, link, n1, n2);
                                    break;
                                case valveType_t::PSV:
                                    psvcoeff(pr, link, n1, n2);
                                    break;
                                case valveType_t::FCV:
                                    fcvcoeff(pr, link, n1, n2);
                                    break;
                                default:   continue;
                                }
                            }
                        }
                    }
                };
                static void   matrixcoeffs(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **  Input:   none
                    **  Output:  none
                    **  Purpose: computes coefficients of linearized network eqns.
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    // Reset values of all diagonal coeffs. (Aii), off-diagonal coeffs. (Aij), r.h.s. coeffs. (F) and node excess flow (Xflow)
                    hyd->smatrix->Aii.Clear();
                    hyd->smatrix->Aij.Clear();
                    hyd->smatrix->F.Clear();

                    hyd->smatrix->Aii.AssureSize(net->nodes.Num() + 1);
                    hyd->smatrix->Aij.AssureSize(hyd->smatrix->Ncoeffs + 1);
                    hyd->smatrix->F.AssureSize(net->nodes.Num() + 1);

                    // Compute matrix coeffs. from links, emitters, and nodal demands
                    linkcoeffs(pr);
                    emittercoeffs(pr);
                    demandcoeffs(pr);

                    // Update nodal flow balances with demands and add onto r.h.s. coeffs.
                    nodecoeffs(pr);

                    // Finally, find coeffs. for PRV/PSV/FCV control valves whose status is not fixed to OPEN/CLOSED
                    valvecoeffs(pr);
                };
                static bool  badvalve(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeNode> n)
                    /*
                    **-----------------------------------------------------------------
                    **  Input:   n = node index
                    **  Output:  returns 1 if node n belongs to an active control valve,
                    **           0 otherwise
                    **  Purpose: determines if a node belongs to an active control valve
                    **           whose setting causes an inconsistent set of eqns. If so,
                    **           the valve status is fixed open and a warning condition
                    **           is generated.
                    **-----------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    for (auto& valve : net->valves) {
                        AUTO n1 = valve.StartingAsset;
                        AUTO n2 = valve.EndingAsset;

                        if (n == n1 || n == n2)
                        {
                            AUTO t = valve.valveType.Read();
                            if (t == valveType_t::PRV || t == valveType_t::PSV || t == valveType_t::FCV)
                            {
                                AUTO link = net->FindLink(valve.Name);
                                auto& link_status = LinkStatus(pr, link);
                                if (link_status == Scalar(::epanet::ACTIVE))
                                {
                                    if (t == valveType_t::FCV)                                        
                                        link_status = Scalar(::epanet::XFCV);
                                    else                   
                                        link_status = Scalar(::epanet::XPRESSURE);

                                    return true;
                                }
                            }
                            return false;
                        }
                    }
                    return false;
                };


                typedef struct {
                    foot_t maxheaderror;
                    cubic_foot_per_second_t maxflowerror;
                    cubic_foot_per_second_t maxflowchange;
                    cweeSharedPtr<cweeLink>    maxheadlink;
                    cweeSharedPtr<cweeNode>    maxflownode;
                    cweeSharedPtr<cweeLink>    maxflowlink;
                } Hydbalance;

                static void  newlinkflows(cweeSharedPtr<Project> pr, Hydbalance* hbal, cubic_foot_per_second_t* qsum, cubic_foot_per_second_t* dqsum)
                    /*
                    **----------------------------------------------------------------
                    **  Input:   hbal = ptr. to hydraulic balance information
                    **           qsum = sum of current system flows
                    **           dqsum = sum of system flow changes
                    **  Output:  updates hbal, qsum and dqsum
                    **  Purpose: updates link flows after new nodal heads computed
                    **----------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    foot_t                  dh;                    /* Link head loss       */
                    cubic_foot_per_second_t dq;                    /* Link flow change     */

                    // Initialize net inflows (i.e., demands) at fixed grade nodes
                    for (auto& res : net->nodes) {
                        if (res->Type == asset_t::RESERVOIR) {
                            NodeDemand(pr, res) = 0.0;
                        }
                    }

                    // Examine each link
                    for (auto& link : net->links) {
                        AUTO n1 = link->StartingAsset;
                        AUTO n2 = link->EndingAsset;

                        // Apply flow update formula:
                        //   dq = Y - P * (new head loss)
                        //    P = 1 / (previous head loss gradient)
                        //    Y = P * (previous head loss)
                        // where P & Y were computed in hlosscoeff() in hydcoeffs.c

                        dh = NodeHead(pr, n1) - NodeHead(pr, n2);
                        dq = Y(pr, link) - P(pr, link) * dh;

                        if (link->Type == asset_t::PUMP) {
                            std::cout << cweeStr::printf("\tWaW PUMP %s\n\t\t", link->Name->c_str());
                            std::cout << "Diameter: " << link->Diameter.Read() << ", ";
                            std::cout << "Kc_Roughness: " << link->Kc_Roughness.Read() << ", ";
                            std::cout << "Km_MinorLoss: " << link->Km_MinorLoss.Read() << ", ";
                            std::cout << "Length: " << link->Length.Read() << ";";
                            std::cout << "\n\t\t";
                            std::cout << "old P: " << P(pr, link) << ", ";
                            std::cout << "old Y: " << Y(pr, link) << ", ";
                            std::cout << "new dh: " << dh << ", ";
                            std::cout << "new dq: " << dq << ";";
                            std::cout << std::endl;
                        }

                        // Adjust flow change by the relaxation factor
                        dq *= Scalar(hyd->RelaxFactor);

                        auto& linkFlow = LinkFlow(pr, link);

                        // Prevent flow in constant HP pumps from going negative
                        if (link->Type == asset_t::PUMP)
                        {
                            AUTO pump = net->pumps.Find(link->Hash());
                            if (pump && pump->HeadCurveMode == ::epanet::CONST_HP && dq > linkFlow)
                            {
                                dq = linkFlow / Scalar(2.0);
                            }
                        }

                        // Update link flow and system flow summation
                        linkFlow -= dq;
                        *qsum += units::math::fabs(linkFlow);
                        *dqsum += units::math::fabs(dq);

                        // Update identity of element with max. flow change
                        if (units::math::fabs(dq) > hbal->maxflowchange)
                        {
                            hbal->maxflowchange = units::math::fabs(dq);
                            hbal->maxflowlink = link;
                            hbal->maxflownode = nullptr;
                        }

                        // Update net flows to fixed grade nodes
                        if (LinkStatus(pr, link) > Scalar(::epanet::CLOSED))
                        {
                            if (n1->Type == asset_t::RESERVOIR) NodeDemand(pr, n1) -= linkFlow;
                            if (n2->Type == asset_t::RESERVOIR) NodeDemand(pr, n2) += linkFlow;
                        }
                    }
                };
                static void newemitterflows(cweeSharedPtr<Project> pr, Hydbalance* hbal, cubic_foot_per_second_t* qsum, cubic_foot_per_second_t* dqsum)
                    /*
                    **----------------------------------------------------------------
                    **  Input:   hbal = ptr. to hydraulic balance information
                    **           qsum = sum of current system flows
                    **           dqsum = sum of system flow changes
                    **  Output:  updates hbal, qsum and dqsum
                    **  Purpose: updates nodal emitter flows after new nodal heads computed
                    **----------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    int     i;
                    foot_t  hloss;
                    ft_per_cfs_t  hgrad;
                    foot_t  dh;
                    cubic_foot_per_second_t  dq;

                    // Examine each network junction
                    for (auto& node : net->nodes) {
                        if (node->Type == asset_t::JUNCTION) {
                            AUTO junc = net->junctions.Find(node->Hash());

                            // Skip junction if it does not have an emitter
                            if (junc->Ke == 0.0) continue;

                            // Find emitter head loss and gradient
                            emitterheadloss(pr, *junc, hloss, hgrad);

                            // Find emitter flow change
                            dh = NodeHead(pr, node) - (foot_t)junc->Coordinates.GetExclusive()->z;

                            dq = (hloss - dh) / hgrad;
                            dq *= Scalar(hyd->RelaxFactor);
                            EmitterFlow(pr, node) -= dq;

                            // Update system flow summation
                            *qsum += units::math::fabs(EmitterFlow(pr, node));
                            *dqsum += units::math::fabs(dq);

                            // Update identity of element with max. flow change
                            if (units::math::fabs(dq) > hbal->maxflowchange)
                            {
                                hbal->maxflowchange = units::math::fabs(dq);
                                hbal->maxflownode = node;
                                hbal->maxflowlink = nullptr;
                            }
                        }
                    }
                };
                static void newdemandflows(cweeSharedPtr<Project> pr, Hydbalance* hbal, cubic_foot_per_second_t* qsum, cubic_foot_per_second_t* dqsum)
                    /*
                    **----------------------------------------------------------------
                    **  Input:   hbal = ptr. to hydraulic balance information
                    **           qsum = sum of current system flows
                    **           dqsum = sum of system flow changes
                    **  Output:  updates hbal, qsum and dqsum
                    **  Purpose: updates nodal pressure dependent demand flows after
                    **           new nodal heads computed
                    **----------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    foot_t dp; // pressure range over which demand can vary (ft)
                    cubic_foot_per_second_t dq; // change in demand flow (cfs)
                    double n; // exponent in head loss v. demand  function
                    foot_t hloss; // current head loss through outflow junction (ft)
                    ft_per_cfs_t hgrad; // head loss gradient with respect to flow (ft/cfs)
                    foot_t dh; // new head loss through outflow junction (ft)

                    // Get demand function parameters
                    if (hyd->DemandModel == ::epanet::DDA) return;

                    dp = units::math::fmax((foot_t)(double)(head_t)(hyd->Preq - hyd->Pmin), (foot_t)::epanet::MINPDIFF);
                    n = 1.0 / hyd->Pexp;

                    // Examine each junction
                    for (auto& node : net->nodes) {
                        if (node->Type == asset_t::JUNCTION) {
                            AUTO junc = net->junctions.Find(node->Hash());

                            // Skip junctions with no positive demand
                            if (NodeDemand(pr, node) <= 0.0_cfs) continue;

                            // Find change in demand flow (see hydcoeffs.c)
                            demandheadloss(pr, *junc, dp, n, hloss, hgrad);
                            dh = NodeHead(pr, node) - ((foot_t)junc->Coordinates.GetExclusive()->z) - (foot_t)(double)(head_t)hyd->Pmin;
                            dq = (hloss - dh) / hgrad;
                            dq *= Scalar(hyd->RelaxFactor);
                            DemandFlow(pr, node) -= dq;

                            // Update system flow summation
                            *qsum += units::math::fabs(DemandFlow(pr, node));
                            *dqsum += units::math::fabs(dq);

                            // Update identity of element with max. flow change
                            if (units::math::fabs(dq) > hbal->maxflowchange)
                            {
                                hbal->maxflowchange = units::math::fabs(dq);
                                hbal->maxflownode = node;
                                hbal->maxflowlink = nullptr;
                            }
                        }
                    }
                };

                static scalar_t newflows(cweeSharedPtr<Project> pr, Hydbalance* hbal)
                    /*
                    **----------------------------------------------------------------
                    **  Input:   hbal = ptr. to hydraulic balance information
                    **  Output:  returns solution convergence error
                    **  Purpose: updates link, emitter & demand flows after new
                    **           nodal heads are computed.
                    **----------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    cubic_foot_per_second_t  dqsum,                 // Network flow change
                        qsum;                  // Network total flow

                    // Initialize sum of flows & corrections
                    qsum = 0.0_gpm;
                    dqsum = 0.0_gpm;
                    hbal->maxflowchange = 0.0;
                    hbal->maxflowlink = nullptr; // 1?
                    hbal->maxflownode = nullptr;

                    // Update flows in all real and virtual links
                    newlinkflows(pr, hbal, &qsum, &dqsum);
                    newemitterflows(pr, hbal, &qsum, &dqsum);
                    newdemandflows(pr, hbal, &qsum, &dqsum);

                    // Return ratio of total flow corrections to total flow
                    if (qsum > hyd->Hacc) return dqsum / qsum;
                    else return Scalar(dqsum);
                };

                static ::epanet::StatusType   next_prv_status(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> link, ::epanet::StatusType s, foot_t  hset, foot_t  h1, foot_t  h2)
                    /*
                    **-----------------------------------------------------------
                    **  Input:   k    = link index
                    **           s    = current status
                    **           hset = valve head setting
                    **           h1   = head at upstream node
                    **           h2   = head at downstream node
                    **  Output:  returns new valve status
                    **  Purpose: updates status of a pressure reducing valve.
                    **-----------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    ::epanet::StatusType status;             // Valve's new status
                    foot_t  hml;                   // Head loss when fully opened
                    foot_t  htol;

                    htol = hyd->Htol;

                    // Head loss when fully open
                    hml = (double)Scalar(link->Km_MinorLoss.Read() * ::epanet::SQR(LinkFlow(pr, link)));

                    AUTO linkflow = LinkFlow(pr, link);

                    // Rules for updating valve's status from current value s
                    status = s;
                    switch (s)
                    {
                    case ::epanet::StatusType::ACTIVE:
                        if (linkflow < -hyd->Qtol)  status = ::epanet::StatusType::CLOSED;            // if the valve was active, but the flow is small, CLOSE the valve.
                        else if (h1 - hml < hset - htol)     status = ::epanet::StatusType::OPEN;             // if the valve was active, flow is not small, but (upstream head - (headloss@fullyOpen)) < desiredHead) then simply set the valve to OPEN. 
                        else                                 status = ::epanet::StatusType::ACTIVE;           // if the valve is ACTIVE and headloss is available, the the flow should be considered ACTIVE. 
                        break;
                    case ::epanet::StatusType::OPEN:
                        if (linkflow < -hyd->Qtol)  status = ::epanet::StatusType::CLOSED;            // if the valve was fully open, but the flow is small, close the valve.
                        else if (h2 >= hset + htol)          status = ::epanet::StatusType::ACTIVE;           // if the valve was fully open, and some excess head is available to blow off, then set to ACTIVE
                        else                                 status = ::epanet::StatusType::OPEN;             // if the valve was fully open, but still not enough head is available for blowoff, then set to OPEN
                        break;
                    case ::epanet::StatusType::CLOSED:
                        if (h1 >= hset + htol && h2 < hset - htol)   status = ::epanet::StatusType::ACTIVE;   // if the valve was closed, and upstream is greater than my setting, and downstread is lower than my setting, then ACTIVATE. 
                        else if (h1 < hset - htol && h1 > h2 + htol) status = ::epanet::StatusType::OPEN;     // if the valve was closed, and the head upstream is less than my setting, yet the upstream is still greater than downstream, then FULLY OPEN
                        else                                         status = ::epanet::StatusType::CLOSED;   // if the valve was closed but the pressure conditions aren't good enough to open or activate, then remain closed. 
                        break;
                    case ::epanet::StatusType::XPRESSURE:
                        if (linkflow < -hyd->Qtol) status = ::epanet::StatusType::CLOSED;             // the valve could not provide the requested pressure and must close. 
                        break;
                    default:
                        // do nothing -> status remains the same. 
                        break;
                    }
                    return status;
                };
                static ::epanet::StatusType   next_psv_status(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> link, ::epanet::StatusType s, foot_t  hset, foot_t  h1, foot_t  h2)
                    /*
                    **-----------------------------------------------------------
                    **  Input:   k    = link index
                    **           s    = current status
                    **           hset = valve head setting
                    **           h1   = head at upstream node
                    **           h2   = head at downstream node
                    **  Output:  returns new valve status
                    **  Purpose: updates status of a pressure sustaining valve.
                    **-----------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    ::epanet::StatusType status;             // Valve's new status
                    foot_t  hml;                   // Head loss when fully opened
                    foot_t  htol;
                    htol = hyd->Htol;

                    AUTO linkflow = LinkFlow(pr, link);

                    // Head loss when fully open
                    hml = (double)Scalar(link->Km_MinorLoss.Read() * ::epanet::SQR(LinkFlow(pr, link)));

                    // Rules for updating valve's status from current value s
                    status = s;
                    switch (s)
                    {
                    case ::epanet::StatusType::ACTIVE:
                        if (linkflow < -hyd->Qtol)              status = ::epanet::StatusType::CLOSED;
                        else if (h2 + hml > hset + htol)                status = ::epanet::StatusType::OPEN;
                        else                                            status = ::epanet::StatusType::ACTIVE;
                        break;

                    case ::epanet::StatusType::OPEN:
                        if (linkflow < -hyd->Qtol)              status = ::epanet::StatusType::CLOSED;
                        else if (h1 < hset - htol)                      status = ::epanet::StatusType::ACTIVE;
                        else                                            status = ::epanet::StatusType::OPEN;
                        break;

                    case ::epanet::StatusType::CLOSED:
                        if (h2 > hset + htol && h1 > h2 + htol)         status = ::epanet::StatusType::OPEN;
                        else if (h1 >= hset + htol && h1 > h2 + htol)   status = ::epanet::StatusType::ACTIVE;
                        else                                            status = ::epanet::StatusType::CLOSED;
                        break;

                    case ::epanet::StatusType::XPRESSURE:
                        if (linkflow < -hyd->Qtol) status = ::epanet::StatusType::CLOSED;
                        break;

                    default:
                        break;
                    }
                    return status;
                };
                static int          calc_and_set_prv_and_psv_status(cweeSharedPtr<Project> pr)
                    /*
                    **-----------------------------------------------------------------
                    **  Input:   none
                    **  Output:  returns 1 if any pressure or flow control valve
                    **           changes status, 0 otherwise
                    **  Purpose: updates status for PRVs & PSVs whose status
                    **           is not fixed to OPEN/CLOSED
                    **-----------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    int    change = FALSE;   // Status change flag
                    ::epanet::StatusType status;       // Valve status settings

                    // Examine each valve
                    for (auto& link : net->links) {
                        if (link->Type == asset_t::VALVE) {
                            AUTO valve = net->valves.Find(link->Hash());
                            if (LinkSetting(pr, link) == ::epanet::MISSING) continue;

                            // Get start/end node indexes & save current status
                            AUTO n1 = link->StartingAsset;
                            AUTO n2 = link->EndingAsset;
                            status = (::epanet::StatusType)(double)LinkStatus(pr, link);

                            // Evaluate valve's new status
                            switch (valve->valveType) {
                            case valveType_t::PRV: {
                                    foot_t hset = (foot_t)n2->Coordinates.GetExclusive()->z + (foot_t)(double)LinkSetting(pr, link);
                                    LinkStatus(pr, link) = next_prv_status(pr, link, status, hset, NodeHead(pr, n1), NodeHead(pr, n2));
                                }
                                break;
                            case valveType_t::PSV: {
                                    foot_t hset = (foot_t)n1->Coordinates.GetExclusive()->z + (foot_t)(double)LinkSetting(pr, link);
                                    LinkStatus(pr, link) = next_psv_status(pr, link, status, hset, NodeHead(pr, n1), NodeHead(pr, n2));
                                }
                                break;
                            default:
                                continue;
                            }

                            // Check for a status change
                            if (Scalar(status) != LinkStatus(pr, link))
                            {
                                change = TRUE;
                            }
                        }
                    }
                    return change;
                };

                static void  checkhydbalance(cweeSharedPtr<Project> pr, Hydbalance* hbal)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   hbal = hydraulic balance errors
                    **   Output:  none
                    **   Purpose: finds the link with the largest head imbalance
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;


                    hbal->maxheaderror = 0.0;
                    hbal->maxheadlink = nullptr;
                    headlosscoeffs(pr);
                    for (auto& link : net->links) {
                        auto s = LinkStatus(pr, link);
                        auto p = P(pr, link);
                        if (s <= (decltype(s))::epanet::CLOSED) continue;
                        if (p == (decltype(p))0) continue;

                        AUTO n1 = link->StartingAsset;
                        AUTO n2 = link->EndingAsset;
                        auto dh = NodeHead(pr, n1) - NodeHead(pr, n2);
                        auto headloss = Y(pr, link) / P(pr, link);
                        auto headerror = units::math::fabs(dh - headloss);
                        if (headerror > (decltype(headerror))hbal->maxheaderror)
                        {
                            hbal->maxheaderror = headerror;
                            hbal->maxheadlink = link;
                        }
                    }
                };
                static int pdaconverged(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   none
                    **   Output:  returns 1 if PDA converged, 0 if not
                    **   Purpose: checks if pressure driven analysis has converged
                    **            and updates total demand deficit
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    constexpr cubic_foot_per_second_t TOL = 0.001;
                    constexpr foot_t TOL2 = 0.001;
                    int i, converged = 1;
                    cubic_foot_per_second_t totalDemand = 0.0;
                    cubic_foot_per_second_t totalReduction = 0.0;

                    hyd->DeficientNodes = 0;
                    hyd->DemandReduction = 0.0;

                    // Add up number of junctions with demand deficits
                    for (auto& node : net->nodes) {
                        if (node->Type == asset_t::JUNCTION) {
                            AUTO junc = net->junctions.Find(node->Hash());

                            // Skip nodes whose required demand is non-positive
                            if (NodeDemand(pr, node) <= 0.0_cfs) continue;

                            // Check for negative demand flow or positive demand flow at negative pressure
                            if (DemandFlow(pr, node) < -TOL) converged = 0;
                            if (DemandFlow(pr, node) > TOL && (NodeHead(pr, node) - (foot_t)node->Coordinates.GetExclusive()->z - (foot_t)(double)(head_t)hyd->Pmin) < -TOL2)
                                converged = 0;

                            // Accumulate total required demand and demand deficit
                            if (DemandFlow(pr, node) + 0.0001_cfs < NodeDemand(pr, node))
                            {
                                hyd->DeficientNodes++;
                                totalDemand += NodeDemand(pr, node);
                                totalReduction += NodeDemand(pr, node) - DemandFlow(pr, node);
                            }
                        }
                    }

                    if (totalDemand > 0.0_cfs) hyd->DemandReduction = (totalReduction / totalDemand) * 100.0;
                    return converged;
                };
                static int  hasconverged(cweeSharedPtr<Project> pr, scalar_t* relerr, Hydbalance* hbal)
                    /*
                    **--------------------------------------------------------------
                    **   Input:   relerr = current total relative flow change
                    **            hbal   = current hydraulic balance errors
                    **   Output:  returns 1 if system has converged or 0 if not
                    **   Purpose: checks various criteria to see if system has
                    **            become hydraulically balanced
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    // Check that total relative flow change is small enough
                    if (*relerr > Scalar(hyd->Hacc)) return 0;

                    // Find largest head loss error and absolute flow change
                    checkhydbalance(pr, hbal);

                    // Check that head loss error and flow change criteria are met
                    if (hyd->HeadErrorLimit > 0.0_ft && hbal->maxheaderror > hyd->HeadErrorLimit) return 0;
                    if (hyd->FlowChangeLimit > 0.0_cfs && hbal->maxflowchange > hyd->FlowChangeLimit) return 0;

                    // Check for pressure driven analysis convergence
                    if (hyd->DemandModel == ::epanet::PDA) return pdaconverged(pr);
                    return 1;
                };


                static ::epanet::StatusType  cvstatus(cweeSharedPtr<Project> pr, ::epanet::StatusType s, foot_t dh, cubic_foot_per_second_t q)
                    /*
                    **--------------------------------------------------
                    **  Input:   s  = current link status
                    **           dh = head loss across link
                    **           q  = link flow
                    **  Output:  returns new link status
                    **  Purpose: updates status of a check valve link.
                    **--------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    // Prevent reverse flow through CVs
                    if (units::math::fabs(dh) > hyd->Htol)
                    {
                        if (dh < -hyd->Htol)     return ::epanet::CLOSED;
                        else if (q < -hyd->Qtol) return ::epanet::CLOSED;
                        else                     return ::epanet::OPEN;
                    }
                    else
                    {
                        if (q < -hyd->Qtol) return ::epanet::CLOSED;
                        else                return s;
                    }
                };
                static ::epanet::StatusType  pumpstatus(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k, foot_t dh)
                    /*
                    **--------------------------------------------------
                    **  Input:   k  = link index
                    **           dh = head gain across link
                    **  Output:  returns new pump status
                    **  Purpose: updates status of an open pump.
                    **--------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    foot_t hmax;

                    // Find maximum head (hmax) pump can deliver
                    auto p = net->pumps.Find(k->Hash());
                    if (p->HeadCurveMode.Read() == ::epanet::CONST_HP) {
                        // Use huge value for constant HP pump
                        hmax = ::epanet::BIG;
                    }
                    else {
                        // Use speed-adjusted shut-off head for other pumps
                        hmax = ::epanet::SQR(LinkSetting(pr, k)) * p->HeadAtFlow(0_cfs);
                    }

                    // Check if currrent head gain exceeds pump's max. head
                    if (dh > hmax + hyd->Htol) return ::epanet::XHEAD;

                    // No check is made to see if flow exceeds pump's max. flow
                    return ::epanet::OPEN;
                };
                static ::epanet::StatusType  next_fcv_status(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k, ::epanet::StatusType s, foot_t h1, foot_t h2)
                    /*
                    **-----------------------------------------------------------
                    **  Input:   k    = link index
                    **           s    = current status
                    **           h1   = head at upstream node
                    **           h2   = head at downstream node
                    **  Output:  returns new valve status
                    **  Purpose: updates status of a flow control valve.
                    **
                    **    Valve status changes to XFCV if flow reversal.
                    **    If current status is XFCV and current flow is
                    **    above setting, then valve becomes active.
                    **    If current status is XFCV, and current flow
                    **    positive but still below valve setting, then
                    **    status remains same.
                    **-----------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    ::epanet::StatusType status;            // New valve status

                    status = s;
                    if (h1 - h2 < -hyd->Htol) // if the upstream head is less than the downstream head, simply exit early and say we cannot do this. 
                    {
                        status = ::epanet::XFCV;
                    }
                    else if (LinkFlow(pr, k) < -hyd->Qtol) // else if the flow through the link is negative, exit early and say we cannot do this.  
                    {
                        status = ::epanet::XFCV;
                    }
                    else if (s == ::epanet::XFCV && (double)LinkFlow(pr, k) >= (double)LinkSetting(pr, k)) // else if the flowis currently in a bad state, see if the flow has returned to the desired range, and set to ACTIVE to clamp the flow. 
                    {
                        status = ::epanet::ACTIVE;
                    }
                    return status;
                };
                static void  tankstatus(cweeSharedPtr<Project> pr, cweeSharedPtr<cweeLink> k, cweeSharedPtr<cweeNode> n1, cweeSharedPtr<cweeNode> n2)
                    /*
                    **----------------------------------------------------------------
                    **  Input:   k  = link index
                    **           n1 = start node of link
                    **           n2 = end node of link
                    **  Output:  none
                    **  Purpose: closes link flowing into full or out of empty tank
                    **----------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    foot_t h;
                    cubic_foot_per_second_t q;


                    // Return if link is closed
                    if (LinkStatus(pr, k) <= Scalar(::epanet::CLOSED)) return;

                    // Make node n1 be the tank, reversing flow (q) if need be
                    q = LinkFlow(pr, k);
                    if (n1->Type == asset_t::RESERVOIR) {
                        // great
                    } else {
                        // inverse
                        AUTO n_temp = n1;
                        n1 = n2;
                        n2 = n1;
                        q *= Scalar(-1);
                    }

                    // Ignore reservoirs
                    AUTO res = net->reservoirs.Find(n1->Hash());
                    if (res && res->TerminalStorage.Read() || !res) return;

                    // Find head difference across link
                    h = NodeHead(pr, n1) - NodeHead(pr, n2);

                    // If tank is full, then prevent flow into it
                    if (NodeHead(pr, n1) >= res->MaxHead() - hyd->Htol && !res->CanOverflow.Read())
                    {
                        // Case 1: Link is a pump discharging into tank
                        if (k->Type == asset_t::PUMP) {
                            if (k->EndingAsset == n1) LinkStatus(pr, k) = Scalar(::epanet::TEMPCLOSED);
                        }
                        // Case 2: Downstream head > tank head (e.g., an open outflow check valve would close)
                        else if (cvstatus(pr, ::epanet::OPEN, h, q) == ::epanet::CLOSED)
                        {
                            LinkStatus(pr, k) = Scalar(::epanet::TEMPCLOSED);
                        }
                    }

                    // If tank is empty, then prevent flow out of it
                    if (NodeHead(pr, n1) <= res->MinHead() + hyd->Htol)
                    {
                        // Case 1: Link is a pump discharging from tank
                        if (k->Type == asset_t::PUMP) {
                            if (k->StartingAsset == n1) LinkStatus(pr, k) = Scalar(::epanet::TEMPCLOSED);
                        }
                        // Case 2: Tank head > downstream head (e.g., a closed outflow check valve would open)
                        else if (cvstatus(pr, ::epanet::CLOSED, h, q) == ::epanet::OPEN)
                        {
                            LinkStatus(pr, k) = Scalar(::epanet::TEMPCLOSED);
                        }
                    }
                };

                static int  linkstatus(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **  Input:   none
                    **  Output:  returns 1 if any link changes status, 0 otherwise
                    **  Purpose: determines new status for pumps, CVs, FCVs & pipes
                    **           to tanks.
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    int change = FALSE;             // Status change flag
                    ::epanet::StatusType  status;             // Current status

                    // Examine each link
                    for (auto& link : net->links) {
                        AUTO n1 = link->StartingAsset;
                        AUTO n2 = link->EndingAsset;
                        AUTO dh = NodeHead(pr, n1) - NodeHead(pr, n2);

                        // Re-open temporarily closed links (status = XHEAD or TEMPCLOSED)
                        status = (::epanet::StatusType)(double)LinkStatus(pr, link);
                        if (status == ::epanet::XHEAD || status == ::epanet::TEMPCLOSED)
                        {
                            LinkStatus(pr, link) = Scalar(::epanet::OPEN);
                        }

                        // Check for status changes in CVs and pumps
                        AUTO pipe = net->Find<asset_t::PIPE>(link->Name);
                        AUTO pump = net->Find<asset_t::PUMP>(link->Name);
                        AUTO valve = net->Find<asset_t::VALVE>(link->Name);

                        if (pipe && pipe->CheckValve.Read() == true)
                        {
                            LinkStatus(pr, link) = cvstatus(pr, (::epanet::StatusType)(double)LinkStatus(pr, link), dh, LinkFlow(pr, link));
                        }
                        else if (pump && LinkStatus(pr, link) >= Scalar(::epanet::OPEN) && LinkSetting(pr, link) > 0.0)
                        {
                            LinkStatus(pr, link) = pumpstatus(pr, link, -dh);
                        }
                        // Check for status changes in non-fixed FCVs
                        else if (valve && valve->valveType == valveType_t::FCV && LinkSetting(pr, link) != ::epanet::MISSING)
                        {
                            LinkStatus(pr, link) = next_fcv_status(pr, link, status, NodeHead(pr, n1), NodeHead(pr, n2));
                        }

                        // Check for flow into (out of) full (empty) tanks
                        if (n1->Type == asset_t::RESERVOIR || n2->Type == asset_t::RESERVOIR)
                        {
                            tankstatus(pr, link, n1, n2);
                        }

                        // Note any change in link status; do not revise link flow
                        if (Scalar(status) != LinkStatus(pr, link))
                        {
                            change = TRUE;
                        }
                    }

                    return change;
                };
                static int  pswitch(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **  Input:   none
                    **  Output:  returns 1 if status of any link changes, 0 if not
                    **  Purpose: adjusts settings of links controlled by junction
                    **           pressures after a hydraulic solution is found
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    AUTO time = pr->times;

                    int   i,                 // Control statement index
                        k,                 // Index of link being controlled
                        n,                 // Node controlling link k
                        reset,             // Flag on control conditions
                        change,            // Flag for status or setting change
                        anychange = 0;     // Flag for 1 or more control actions

                    // Check each control statement
                    for (AssetController& controller : net->Controllers) {
                        reset = 0;
                        AUTO link = controller.Parent;
                        if (!link) continue;

                        AUTO s = (::epanet::StatusType)(double)LinkStatus(pr, link);
                        AUTO sett = LinkSetting(pr, link);

                        for (cweeSharedPtr< cweeControl > inner_control : *controller.StatusControllers.GetExclusive()) {
                            if (inner_control) {
                                auto observed_assets = inner_control->GetObservedAssets();
                                bool tryThis = false;
                                for (auto& observed_asset : observed_assets) {
                                    if (observed_asset && observed_asset->Type == asset_t::JUNCTION) {
                                        auto junc = net->junctions.Find(observed_asset->Hash());
                                        // update the junction's characteristics for the control to see
                                        observed_asset->GetValue<_HEAD_>()->AddUniqueValue((u64)time->Htime, NodeHead(pr, net->FindNode(observed_asset->Name)));
                                        observed_asset->GetValue<_DEMAND_>()->AddUniqueValue((u64)time->Htime, junc->DemandAt((u64)time->Htime));
                                        tryThis = true;
                                    }
                                }
                                if (tryThis) {
                                    auto ptr = inner_control->TryGetNextSetting((u64)time->Htime, (u64)(time->Htime - (double)time->Hstep));
                                    if (ptr) {
                                        AUTO p = link->GetValue<_STATUS_>();
                                        if (p) {
                                            p->AddUniqueValue((u64)time->Htime, *ptr);
                                            LinkStatus(pr, link) = *ptr;
                                        }
                                    }
                                }
                            }
                        }
                        for (cweeSharedPtr< cweeControl > inner_control : *controller.SettingControllers.GetExclusive()) {
                            if (inner_control) {
                                auto observed_assets = inner_control->GetObservedAssets();
                                bool tryThis = false;
                                for (auto& observed_asset : observed_assets) {
                                    if (observed_asset && observed_asset->Type == asset_t::JUNCTION) {
                                        auto junc = net->junctions.Find(observed_asset->Hash());
                                        // update the junction's characteristics for the control to see
                                        observed_asset->GetValue<_HEAD_>()->AddUniqueValue((u64)time->Htime, NodeHead(pr, net->FindNode(observed_asset->Name)));
                                        observed_asset->GetValue<_DEMAND_>()->AddUniqueValue((u64)time->Htime, junc->DemandAt((u64)time->Htime));
                                        tryThis = true;
                                    }
                                }
                                if (tryThis) {
                                    auto ptr = inner_control->TryGetNextSetting((u64)time->Htime, (u64)(time->Htime - (double)time->Hstep));
                                    if (ptr) {
                                        AUTO p = link->GetValue<_SETTING_>();
                                        if (p) {
                                            p->AddUniqueValue((u64)time->Htime, *ptr);
                                            LinkSetting(pr, link) = *ptr;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    return anychange;
                };

                static int  hydsolve(cweeSharedPtr<Project> pr, int* iter, scalar_t* relerr)
                    /*
                    **-------------------------------------------------------------------
                    **  Input:   none
                    **  Output:  *iter   = # of iterations to reach solution
                    **           *relerr = convergence error in solution
                    **           returns error code
                    **  Purpose: solves network nodal equations for heads and flows
                    **           using Todini's Gradient algorithm
                    **
                    **  Notes:   Status checks on CVs, pumps and pipes to tanks are made
                    **           every CheckFreq iteration, up until MaxCheck iterations
                    **           are reached. Status checks on control valves are made
                    **           every iteration if DampLimit = 0 or only when the
                    **           convergence error is at or below DampLimit. If DampLimit
                    **           is > 0 then future computed flow changes are only 60% of
                    **           their full value. A complete status check on all links
                    **           is made when convergence is achieved. If convergence is
                    **           not achieved in MaxIter trials and ExtraIter > 0 then
                    **           another ExtraIter trials are made with no status changes
                    **           made to any links and a warning message is generated.
                    **
                    **   This procedure calls linsolve() which appears in SMATRIX.C.
                    **-------------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    int    i;                     // Node index
                    int    errcode = 0;           // Node causing solution error
                    int    nextcheck;             // Next status check trial
                    int    maxtrials;             // Max. trials for convergence
                    scalar_t newerr;                // New convergence error
                    int    valveChange;           // Valve status change flag
                    int    statChange;            // Non-valve status change flag
                    Hydbalance hydbal;            // Hydraulic balance errors
                    cubic_foot_per_second_t fullDemand;            // Full demand for a node (cfs)

                    // Initialize status checking & relaxation factor
                    nextcheck = hyd->CheckFreq;
                    hyd->RelaxFactor = 1.0;

                    // Initialize convergence criteria and PDA results
                    hydbal.maxheaderror = 0.0;
                    hydbal.maxflowchange = 0.0;
                    hyd->DeficientNodes = 0;
                    hyd->DemandReduction = 0.0;

                    /* Repeat iterations until convergence or trial limit is exceeded. (ExtraIter used to increase trials in case of status cycling.) */
                    maxtrials = hyd->MaxIter;
                    if (hyd->ExtraIter > 0) maxtrials += hyd->ExtraIter;
                    *iter = 1;
                    while (*iter <= maxtrials) {
                        // Compute coefficient matrices A & F and solve A*H = F where H = heads, A = Jacobian coeffs. derived from head loss gradients, & F = flow correction terms. Solution for H is returned in F from call to linsolve().
                        headlosscoeffs(pr);
                        matrixcoeffs(pr);     
                        errcode = smatrix_t::linsolve(hyd->smatrix, net->junctions.Num()); // errcode = smatrix_t::linsolve(sm, net->Njuncs);

                        // Matrix ill-conditioning problem - if control valve causing problem, fix its status & continue, otherwise quit with no solution.
                        if (errcode > 0) {
                            if (hyd->smatrix->Order.Num() > errcode) {
                                AUTO ptr = smatrix_t::IndexToNode(hyd->smatrix, hyd->smatrix->Order[errcode]);
                                if (ptr && badvalve(pr, ptr)) continue;
                            }
                            break;                            
                        }                        

                        // Update current solution. (Row[i] = row of solution matrix corresponding to node i)
                        for (auto& node : net->nodes) {
                            if (node->Type == asset_t::JUNCTION) 
                                NodeHead(pr, node) = hyd->smatrix->B_ft[hyd->smatrix->Row[smatrix_t::NodeToIndex(hyd->smatrix, node)]];
                        }

                        newerr = newflows(pr, &hydbal);             // Update flows
                        *relerr = newerr;

                        // Apply solution damping & check for change in valve status
                        hyd->RelaxFactor = 1.0;
                        valveChange = FALSE;
                        if (hyd->DampLimit > 0.0)
                        {
                            if (*relerr <= Scalar(hyd->DampLimit))
                            {
                                hyd->RelaxFactor = 0.6;
                                valveChange = calc_and_set_prv_and_psv_status(pr);
                            }
                        }
                        else
                        {
                            valveChange = calc_and_set_prv_and_psv_status(pr);
                        }

                        // Check for convergence
                        if (hasconverged(pr, relerr, &hydbal))
                        {
                            // We have convergence - quit if we are into extra iterations
                            if (*iter > hyd->MaxIter) break;

                            // Quit if no status changes occur
                            statChange = FALSE;
                            if (valveChange)    statChange = TRUE;
                            if (linkstatus(pr)) statChange = TRUE;
                            if (pswitch(pr))    statChange = TRUE;
                            if (!statChange)    break;

                            // We have a status change so continue the iterations
                            nextcheck = *iter + hyd->CheckFreq;
                        }
                        // No convergence yet - see if its time for a periodic status check on pumps, CV's, and pipes connected to tank
                        else if (*iter <= hyd->MaxCheck && *iter == nextcheck)
                        {
                            linkstatus(pr);
                            nextcheck += hyd->CheckFreq;
                        }

                        (*iter)++;
                    }

                    // Iterations ended - report any errors.
                    if (errcode > 0) {
                        // writehyderr(pr, sm->Order[errcode]);    // Ill-conditioned matrix error
                        errcode = 110;
                    }

                    // Store actual junction outflow in NodeDemand & full demand in DemandFlow
                    for (auto& junc : net->junctions) {
                        AUTO n = net->nodes.Find(junc.Hash());
                        if (n) {
                            fullDemand = NodeDemand(pr, *n);
                            NodeDemand(pr, *n) = DemandFlow(pr, *n) + EmitterFlow(pr, *n);
                            DemandFlow(pr, *n) = fullDemand;
                        }
                    }

                    // Save convergence info
                    hyd->RelativeError = *relerr;
                    hyd->MaxHeadError = hydbal.maxheaderror;
                    hyd->MaxFlowChange = hydbal.maxflowchange;
                    hyd->Iterations = *iter;
                    return errcode;
                };



                static void inithyd(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **  Input:   initflag > 0 if link flows should be re-initialized
                    **                    = 0 if not
                    **  Output:  none
                    **  Purpose: initializes hydraulics solver system
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;

                    // Initialize current time
                    hyd->Haltflag = 0;
                    time->Htime = pr->times->GetProjectStartTime();
                    time->Hydstep = 0;
                    time->Rtime = time->Rstep;

                    // Initialize tanks
                    for (auto& res : net->reservoirs) {
                        NodeHead(pr, *net->nodes.Find(res.Hash())) = res.GetCurrentValue<_HEAD_>((u64)pr->times->Htime);
                        NodeDemand(pr, *net->nodes.Find(res.Hash())) = 0_gpm;
                        OldStatus(pr, *net->assets.Find(res.Hash())) = Scalar(::epanet::TEMPCLOSED);
                    }

                    // Initialize emitter flows
                    for (auto& node : net->nodes) {
                        if (node->Ke.Read() > 0.0) EmitterFlow(pr, node) = 1.0;
                    }

                    // Initialize links
                    for (auto& link : net->links) {
                        LinkStatus(pr, link) = link->GetCurrentValue<_STATUS_>((u64)time->Htime);
                        LinkSetting(pr, link) = link->Kc_Roughness.Read();

                        resistcoeff(pr, link);

                        AUTO valve = net->valves.Find(link->Hash());
                        if (valve) { // Start active control valves in ACTIVE position
                            if (
                                (valve->valveType == valveType_t::PRV || valve->valveType == valveType_t::PSV
                                    || valve->valveType == valveType_t::FCV) && (valve->Kc_Roughness.Read() != Scalar(::epanet::MISSING))
                                ) LinkStatus(pr, link) = Scalar(::epanet::ACTIVE);
                        }

                        // Initialize flows if necessary
                        if (LinkStatus(pr, link) <= Scalar(::epanet::CLOSED))
                        {
                            LinkFlow(pr, link) = ::epanet::QZERO;
                        }
                        else // if (units::math::fabs(LinkFlow(pr, link)) <= Scalar(::epanet::QZERO) || initflag > 0)
                        {
                            initlinkflow(pr, link);
                        }
                        // Save initial status
                        OldStatus(pr, link.CastReference<cweeAsset>()) = LinkStatus(pr, link);
                    }
                };

                static void     convertunits(cweeSharedPtr<Project> pr)
                    /*
                    **--------------------------------------------------------------
                    **  Input:   none
                    **  Output:  none
                    **  Purpose: converts units of input data
                    **--------------------------------------------------------------
                    */
                {
                    AUTO net = pr->network;
                    AUTO hyd = pr->hydraul;
                    
                    AUTO time = pr->times;
                    AUTO parser = pr->parser;

                    int i, j, k;
                    double ucf;     // Unit conversion factor

                    // Convert PDA pressure limits
                    // hyd->Pmin /= pr->Ucf[PRESSURE];
                    // hyd->Preq /= pr->Ucf[PRESSURE];

                    // Convert emitter discharge coeffs. to head loss coeff.
                    //ucf = pow(pr->Ucf[FLOW], hyd->Qexp) / pr->Ucf[PRESSURE];
                    //for (i = 1; i <= net->Njuncs; i++)
                    //{
                    //    node = &net->Node[i];
                    //    if (node->Ke > 0.0) node->Ke = ucf / pow(node->Ke, hyd->Qexp);
                    //}

                    // Convert water quality concentration options
                    // qual->Climit /= pr->Ucf[QUALITY];
                    // qual->Ctol /= pr->Ucf[QUALITY];

                    // Convert global reaction coeffs.
                    //qual->Kbulk /= SECperDAY;
                    //qual->Kwall /= SECperDAY;

                    // Convert units of link parameters
                    for (auto& link : net->links) {
                        if (link->Type == asset_t::PIPE) {
                            // Convert D-W roughness from millifeet (or mm) to ft
                            if (hyd->Formflag == ::epanet::DW) link->Kc_Roughness = link->Kc_Roughness.Read() / 1000.0;

                            // Convert minor loss coeff. from V^2/2g basis to Q^2 basis
                            link->Km_MinorLoss = Scalar(0.02517) * Scalar(link->Km_MinorLoss.Read()) / Scalar(::epanet::SQR(link->Diameter.Read())) / Scalar(::epanet::SQR(link->Diameter.Read()));

                            // Convert units on reaction coeffs.
                            link->Kb_BulkReactionCoeff = link->Kb_BulkReactionCoeff.Read() / ::epanet::SECperDAY;
                            link->Kw_WallReactionCoeff = link->Kw_WallReactionCoeff.Read() / ::epanet::SECperDAY;
                        }
                        else if (link->Type == asset_t::PUMP) {
                            AUTO pump = net->pumps.Find(link->Hash());
                            if (pump) {
                                if (pump->HeadCurveMode == ::epanet::CONST_HP)
                                {
                                    // For constant hp pump, convert kw to hp
                                    if (parser->Unitsflag == ::epanet::SI) 
                                        pump->R_FlowResistance = (double)((horsepower_t)((kilowatt_t)((double)(pump->R_FlowResistance.Read()))));
                                }
                                else
                                {
                                    // For power curve pumps, convert shutoff head and flow coeff.
                                    if (pump->HeadCurveMode == ::epanet::POWER_FUNC) {
                                        // pump->H0 /= pr->Ucf[HEAD];

                                        std::cout << "CANNOT SUPPORT POWER FUNCTION PUMP DUE TO UNITS HANDLING ISSUE" << std::endl;                                        
                                        //pump->R_FlowResistance *= (pow(pr->Ucf[FLOW], pump->N) / pr->Ucf[HEAD]);
                                    }
                                }
                            }
                        }
                        else if (link->Type == asset_t::VALVE) {
                            AUTO valve = net->valves.Find(link->Hash());
                            if (valve) {
                                // For flow control valves, convert flow setting while for other valves convert pressure setting
                                
                                // Convert minor loss coeff. from V^2/2g basis to Q^2 basis
                                link->Km_MinorLoss = Scalar(0.02517) * Scalar(link->Km_MinorLoss.Read()) / Scalar(::epanet::SQR(link->Diameter.Read())) / Scalar(::epanet::SQR(link->Diameter.Read()));

                                if (link->Kc_Roughness.Read() != ::epanet::MISSING) switch (valve->valveType)
                                {
                                case valveType_t::FCV:
                                    // EPAnet stored the desired setting in the roughness parameter... we're now storing it in the time-series. 
                                    //// link->Kc_Roughness /= pr->Ucf[FLOW];
                                    break;
                                case valveType_t::PRV:
                                case valveType_t::PSV:
                                case valveType_t::PBV:
                                    // EPAnet stored the desired setting in the roughness parameter... we're now storing it in the time-series.                                     
                                    //// link->Kc_Roughness /= pr->Ucf[PRESSURE];
                                    break;
                                default:
                                    break;
                                }

                            }




                        }
                    }
                };
























				static void runhyd(cweeSharedPtr<Project> pr) {
					AUTO net = pr->network;
					AUTO hyd = pr->hydraul;
					AUTO time = pr->times;

					int   iter = 0;          // Iteration count
					int   errcode = 0;       // Error code
					scalar_t relerr = 0;       // Solution accuracy

					// Find new demands & control actions
					demands(pr);
					controls(pr);

					/* Solve network hydraulic equations */
					errcode = hydsolve(pr, &iter, &relerr);
					if (!errcode)
					{
						// If system unbalanced and no extra trials allowed, then activate the Haltflag
						if (relerr > Scalar(hyd->Hacc) && hyd->ExtraIter == -1)
						{
							hyd->Haltflag = 1;
						}

						// Report any warning conditions
						/* if (!errcode) errcode = writehydwarn(pr, iter, relerr); */
					}
				};



                static void		startHydraulicSim(cweeSharedPtr<Project> p) { openhyd(p); inithyd(p); convertunits(p); };
				static void		calcHydraulicsAtTime(cweeSharedPtr<Project> p) {
					runhyd(p);
				};
				static second_t		stepHydraulicSim(cweeSharedPtr<Project> p) { return nexthyd(p); };
				static void		endHydraulicSim(cweeSharedPtr<Project> p) {};


			};

			class EPAnetHydraulicSimulation {
			public:
				EPAnetHydraulicSimulation(cweeSharedPtr<Project> p) : parent(p) { HydraulicSim::startHydraulicSim(parent); };
				~EPAnetHydraulicSimulation() { HydraulicSim::endHydraulicSim(parent); };
				//void SetTimestep(float minutes) { parent.setTimeParam(5, 60.0f * minutes); };
				//AUTO GetCurrentSimTime() { return parent.getCurrentSimTime(); };
				void DoSteadyState() { HydraulicSim::calcHydraulicsAtTime(parent); };
				bool ShouldContinueSimulation() const {	
					AUTO step = HydraulicSim::stepHydraulicSim(parent);
					if (step >= 0_s) {
						return true;
					}
					else {
						return false;
					}
				};				
				//float GetSimulationValue(cweeStr const& name, value_t const& value_type, bool isNode) { return parent.getCurrentHydraulicValue(name, value_type, isNode); };

			private:
				cweeSharedPtr<Project> parent;

			};
			


		};

		static cweeSharedPtr<details::EPAnetHydraulicSimulation> StartHydraulicSimulation(cweeSharedPtr<Project> p) { return make_cwee_shared<details::EPAnetHydraulicSimulation>(new details::EPAnetHydraulicSimulation(p)); };

	};

};
#endif