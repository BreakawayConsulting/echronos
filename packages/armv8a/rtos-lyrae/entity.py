#
# Copyright (c) 2019, Breakaway Consulting Pty. Ltd.
#
import os.path
from prj import SystemParseError, Module, LengthList


class LyraeModule(Module):
    xml_schema_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'schema.xml')
    files = [
        {'input': 'rtos-lyrae.h', 'render': True},
        {'input': 'rtos-lyrae.c', 'render': True, 'type': 'c'},
    ]

    def configure(self, xml_config):
        config = super().configure(xml_config)

        config['prefix_func'] = config['prefix'] + '_' if config['prefix'] is not None else ''
        config['prefix_type'] = config['prefix'].capitalize() if config['prefix'] is not None else ''
        config['prefix_const'] = config['prefix'].upper() + '_' if config['prefix'] is not None else ''

        # Semi-configurable items
        # These are configurable in the code, but for simplicitly they are not supported as
        # user configuration at this stage.
        config['interrupteventid_size'] = 8
        config['taskid_size'] = 8

        task_id_base = 0
        timer_id_base = 0
        mutex_id_base = 0

        # All object names across taskgroups; used to ensure duplicate names are not used
        all_obj_names = {
            'tasks': {},
            'timers': {},
            'mutexes': {},
        }

        # Ensure that all priorities are unique
        seen_priorities = {}
        for taskgroup in config['taskgroups']:
            if taskgroup['priority'] in seen_priorities:
                template = "Duplicate priority '{}' in taskgroup '{}'. Priority already used in taskgroup '{}'."
                msg = template.format(
                    taskgroup['priority'],
                    taskgroup['name'],
                    seen_priorities[taskgroup['priority']]['name'],
                )
                raise SystemParseError(msg)
            seen_priorities[taskgroup['priority']] = taskgroup


        # Sort by priority and re-index correctly
        config['taskgroups'].sort(key=lambda tg: tg['priority'])
        for idx, t in enumerate(config['taskgroups']):
            t['idx'] = idx

        for taskgroup in config['taskgroups']:
            # Ensure that at least one task is runnable.
            if not any(task['start'] for task in taskgroup['tasks']):
                raise SystemParseError("At least one task must be configured to start.")

            # Calculate the start and end task IDs for the group.
            taskgroup['task_id_base'] = task_id_base
            taskgroup['task_id_max'] = task_id_base + len(taskgroup['tasks']) - 1
            taskgroup['task_id_end'] = task_id_base + len(taskgroup['tasks'])
            task_id_base += len(taskgroup['tasks'])

            # Extend task names
            for task in taskgroup['tasks']:
                #task['name'] = '{}_{}'.format(taskgroup['name'], task['name'])
                task['taskgroup_id'] = taskgroup['idx']

            # Create builtin signals
            # The RTOS task timer signal is used in the following conditions:
            #   1. To notify the task when a mutex is unlocked.
            # The same signal is re-used to avoid excessive allocation of signals.
            taskgroup['signal_labels'].append({'name': '_task_timer', 'global': True})

            # The RTOS utility signal is used to start the task.
            taskgroup['signal_labels'].append({'name': '_rtos_util', 'global': True})

            # Assign signal ids
            sig_sets = []
            for task in taskgroup['tasks']:
                sig_set = []
                for sig in taskgroup['signal_labels']:
                    if sig.get('global', False) or task['name'] in [t['name'] for t in sig['tasks']]:
                        sig_set.append(sig['name'])
                sig_sets.append(sig_set)

            label_ids = assign_signal_vals(sig_sets)
            for sig in taskgroup['signal_labels']:
                sig['idx'] = label_ids[sig['name']]

            # Create signal_set definitions from signal definitions:
            taskgroup['signal_sets'] = [{'name': sig['name'], 'value': 1 << sig['idx'], 'singleton': True}
                                    for sig in taskgroup['signal_labels']]

            signal_set_names = [sigset['name'] for sigset in taskgroup['signal_sets']]

            for interrupt_event in taskgroup['interrupt_events']:
                if interrupt_event['sig_set'] not in signal_set_names:
                    msg = "Unknown signal-set '{}' in interrupt_event '{}'"
                    raise SystemParseError(msg.format(interrupt_event['sig_set'], interrupt_event['name']))

            for timer in taskgroup['timers']:
                if timer['sig_set'] is not None and timer['sig_set'] not in signal_set_names:
                    msg = "Unknown signal-set '{}' in timer '{}'"
                    raise SystemParseError(msg.format(timer['sig_set'], timer['name']))

            # Create a timer for each task
            for task in taskgroup['tasks']:
                timer = {'name': '_task_' + task['name'],
                        'error': 0,
                        'reload': 0,
                        'task': task,
                        'idx': len(taskgroup['timers']),
                        'enabled': False,
                        'sig_set': '_task_timer'}
                task['timer'] = timer
                taskgroup['timers'].append(timer)

            taskgroup['timer_id_base'] = timer_id_base
            taskgroup['timer_id_end'] = timer_id_base + len(taskgroup['timers'])
            timer_id_base += len(taskgroup['timers'])

            taskgroup['mutex_id_base'] = mutex_id_base
            taskgroup['mutex_id_end'] = mutex_id_base + len(taskgroup['mutexes'])
            mutex_id_base += len(taskgroup['mutexes'])

            # Validate names
            for obj_name, singular in zip(('tasks', 'timers', 'mutexes'), ("task", "timer", "mutex")):
                for obj in taskgroup[obj_name]:
                    if obj['name'] in all_obj_names[obj_name]:
                        template = "Duplicate {} name '{}' in taskgroup '{}'. Name already used in taskgroup '{}'."
                        msg = template.format(
                            singular,
                            obj['name'],
                            taskgroup['name'],
                            all_obj_names[obj_name][obj['name']]['name'],
                        )
                        raise SystemParseError(msg)
                    all_obj_names[obj_name][obj['name']] = taskgroup

        # Merge all taskgroup objects into a single list, and re-index.
        for obj_name in ('tasks', 'timers', 'mutexes', 'signal_sets'):
            config[obj_name] = LengthList(sum([tg[obj_name] for tg in config['taskgroups']], []))
            for idx, t in enumerate(config[obj_name]):
                t['idx'] = idx

        return config


def assign_signal_vals(sig_sets):
    """Assign values to each signal in a list of signal sets.

    Values are assigned so that the values in each set are unique.

    A greedy algorithm is used to minimise the signal values used.

    A dictionary of signal values index by signal is returned.

    """
    signals = set().union(*sig_sets)
    possible_vals = set(range(len(signals)))

    assigned = {}
    # Sort the signals so that assign_signal_vals is deterministic based on input.
    # Without sorting Python iterates the signals set in arbitrary order.
    # Note that possibly changes on each Python invocation based on the hashing seed.
    for sig in sorted(signals):
        used_vals = [{assigned.get(ss) for ss in sig_set} for sig_set in sig_sets if sig in sig_set]
        assigned[sig] = min(possible_vals.difference(*used_vals))

    assert all(len({assigned[x] for x in sig_set}) == len(sig_set) for sig_set in sig_sets)

    # Returned dictionary should only be used for lookup, not iteration to ensure overall process is deterministic.
    return assigned


module = LyraeModule()  # pylint: disable=invalid-name
