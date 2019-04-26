(ns editor.fxui
  (:require [cljfx.api :as fx]
            [editor.error-reporting :as error-reporting]
            [editor.ui :as ui]
            [editor.util :as eutil])
  (:import [cljfx.lifecycle Lifecycle]
           [javafx.application Platform]
           [javafx.scene Node]
           [javafx.beans.value ChangeListener]))

(def ext-value
  "Extension lifecycle that returns value on `:value` key"
  (reify Lifecycle
    (create [_ desc _]
      (:value desc))
    (advance [_ _ desc _]
      (:value desc))
    (delete [_ _ _])))

(defn ext-focused-by-default
  "Function component that mimics extension lifecycle. Focuses node specified by
   `:desc` key when it gets added to scene graph"
  [{:keys [desc]}]
  {:fx/type fx/ext-on-instance-lifecycle
   :on-created (fn [^Node node]
                 (if (some? (.getScene node))
                   (.requestFocus node)
                   (.addListener (.sceneProperty node)
                                 (reify ChangeListener
                                   (changed [this _ _ new-scene]
                                     (when (some? new-scene)
                                       (.removeListener (.sceneProperty node) this)
                                       (.requestFocus node)))))))
   :desc desc})

(defn mount-renderer-and-await-result!
  "Mounts `renderer` and blocks current thread until `state-atom`'s value
  receives has a `:result` key"
  [state-atom renderer]
  (let [event-loop-key (Object.)
        result-promise (promise)]
    (future
      (error-reporting/catch-all!
        (let [result @result-promise]
          (fx/on-fx-thread
            (Platform/exitNestedEventLoop event-loop-key result)))))
    (add-watch state-atom event-loop-key
               (fn [k r _ n]
                 (let [result (:result n ::no-result)]
                   (when-not (= result ::no-result)
                     (deliver result-promise result)
                     (remove-watch r k)))))
    (fx/mount-renderer state-atom renderer)
    (Platform/enterNestedEventLoop event-loop-key)))

(defn wrap-state-handler [state-atom f]
  (-> f
      (fx/wrap-co-effects
        {:state (fx/make-deref-co-effect state-atom)})
      (fx/wrap-effects
        {:state (fx/make-reset-effect state-atom)})))

(defn stage [props]
  (assoc props
    :fx/type :stage
    :on-focused-changed ui/focus-change-listener
    :icons (if (eutil/is-mac-os?) [] [ui/application-icon-image])))

(defn dialog-stage [props]
  (assoc props
    :fx/type stage
    :resizable false
    :style :decorated
    :modality (if (contains? props :owner) :window-modal :application-modal)))

(defn- add-style-classes [style-class & classes]
  (let [existing-classes (if (string? style-class) [style-class] style-class)]
    (into existing-classes classes)))

(defn dialog-body [{:keys [size header content footer]
                    :or {size :small}
                    :as props}]
  (-> props
      (dissoc :size :header :content :footer)
      (update :style-class add-style-classes "dialog-body" (case size
                                                             :small "dialog-body-small"
                                                             :large "dialog-body-large"))
      (assoc :fx/type :v-box
             :children (if (some? content)
                         [{:fx/type :v-box
                           :style-class "dialog-with-content-header"
                           :children [header]}
                          {:fx/type :v-box
                           :style-class "dialog-content"
                           :children [content]}
                          {:fx/type :v-box
                           :style-class "dialog-with-content-footer"
                           :children [footer]}]
                         [{:fx/type :v-box
                           :style-class "dialog-without-content-header"
                           :children [header]}
                          {:fx/type :region :style-class "dialog-no-content"}
                          {:fx/type :v-box
                           :style-class "dialog-without-content-footer"
                           :children [footer]}]))))

(defn dialog-buttons [props]
  (-> props
      (assoc :fx/type :h-box)
      (update :style-class add-style-classes "dialog-buttons")))

(defn label [{:keys [variant]
              :or {variant :label}
              :as props}]
  (-> props
      (assoc :fx/type :label)
      (dissoc :variant)
      (update :style-class add-style-classes (case variant
                                               :label "label"
                                               :header "header"))))

(defn button [{:keys [variant]
               :or {variant :secondary}
               :as props}]
  (-> props
      (assoc :fx/type :button)
      (dissoc :variant)
      (update :style-class add-style-classes "button" (case variant
                                                        :primary "button-primary"
                                                        :secondary "button-secondary"))))

(defn two-col-input-grid-pane [props]
  (-> props
      (assoc :fx/type :grid-pane
             :column-constraints [{:fx/type :column-constraints}
                                  {:fx/type :column-constraints
                                   :hgrow :always}])
      (update :style-class add-style-classes "input-grid")
      (update :children (fn [children]
                          (into []
                                (comp
                                  (partition-all 2)
                                  (map-indexed
                                    (fn [row [label input]]
                                      [(assoc label :grid-pane/column 0
                                                    :grid-pane/row row
                                                    :grid-pane/halignment :right)
                                       (assoc input :grid-pane/column 1
                                                    :grid-pane/row row)]))
                                  (mapcat identity))
                                children)))))

(defn text-field [{:keys [variant]
                   :or {variant :default}
                   :as props}]
  (-> props
      (assoc :fx/type :text-field)
      (dissoc :variant)
      (update :style-class add-style-classes "text-field" (case variant
                                                            :default "text-field-default"
                                                            :error "text-field-error"))))

(defn text-area [{:keys [variant]
                  :or {variant :default}
                  :as props}]
  (-> props
      (dissoc :variant)
      (assoc :fx/type :text-area)
      (update :style-class add-style-classes "text-area" (case variant
                                                           :default "text-area-default"
                                                           :error "text-area-error"
                                                           :borderless "text-area-borderless"))))

(defn icon [{:keys [type scale]
             :or {scale 1}}]
  {:fx/type :group
   :children [{:fx/type :svg-path
               :scale-x scale
               :scale-y scale
               :fill (case type
                       :error "#e32f44")
               :content (case type
                          :error "M33.6,27.4L20.1,3.9c-0.6-1-1.6-1.5-2.6-1.5s-2,0.5-2.6,1.5L1.4,27.4C0.2,29.4,1.7,32,4.1,32h26.8 C33.3,32,34.8,29.4,33.6,27.4z M32.7,30c-0.4,0.7-1.1,1.1-1.8,1.1H4.1c-0.8,0-1.4-0.4-1.8-1.1c-0.4-0.7-0.4-1.4,0-2.1L15.8,4.4 c0.4-0.6,1-1,1.7-1s1.3,0.4,1.7,1l13.5,23.4C33.1,28.5,33.1,29.3,32.7,30z M18.1,25.5c0,0.4-0.3,0.7-0.7,0.7s-0.7-0.3-0.7-0.7 c0-0.4,0.3-0.7,0.7-0.7S18.1,25.1,18.1,25.5z M17,22.6v-9.1c0-0.3,0.2-0.5,0.5-0.5s0.5,0.2,0.5,0.5v9.1c0,0.3-0.2,0.5-0.5,0.5 S17,22.9,17,22.6z")}]})
